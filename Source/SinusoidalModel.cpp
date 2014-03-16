/*
  ==============================================================================

    SinusoidalModel.cpp
    Created: 11 Mar 2014 4:50:42pm
    Author:  Owen Campbell

  ==============================================================================
*/

#include "SinusoidalModel.h"
#include "Track.h"

SinusoidalModel::SinusoidalModel(){
    analysis = new Analysis();
    wavetable = new Wavetable<float>();
    
    tracks = new Track[0];
    oscillators = new Oscillator<float>[0];
    matches = new bool[0];
}
SinusoidalModel::~SinusoidalModel(){
    delete analysis;
    delete wavetable;
    delete[] tracks;
    delete[] oscillators;
    delete[] matches;
}

//getters
float * SinusoidalModel::getAnalysisResults(const Analysis::PARAMETER p) const{
    switch (p) {
        case Analysis::PARAMETER::MAG:
            return &analysis->getMagnitudes();
        case Analysis::PARAMETER::PHS:
            return &analysis->getPhases();
        case Analysis::PARAMETER::FRQ:
            return &analysis->getFrequencies();
        default:
            std::cout << "Error: attempting to retrieve analysis results with invalid parameter" << std::endl;
            return nullptr;
    }
}
//setters
void SinusoidalModel::setWaveform(Wavetable<float>::WAVEFORM wf){
    wavetable->setWaveform(wf, false);
}

//business/helper functions
void SinusoidalModel::init(const Analysis::WINDOW w, const int ws, const float sr, const bool p,
          const int wts, Wavetable<float>::WAVEFORM wf){
    windowSize = ws;
    delete wavetable;
    wavetable = new Wavetable<float>(wf, wts);
    analysis->init(w, ws, sr, p);
    maxTracks = analysis->getNumBins();
    
    delete[] tracks;
    tracks = new Track[maxTracks];
    delete[] oscillators;
    oscillators = new Oscillator<float>[maxTracks];
    delete[] matches;
    matches = new bool[maxTracks]{false};
    
    activeTracks = 0;
    //hard coding these for now
    trackBirth = 10;
    trackDeath = 5;
}

bool SinusoidalModel::operator() (const float sample){//use this to write samples to the input buffer
    return analysis->operator()(sample);
}

float SinusoidalModel::operator() (void){//use this to read samples from the output buffer
    return analysis->operator()();//TODO: replace this with output from oscillator bank
}
void SinusoidalModel::transform(const Analysis::TRANSFORM t){
    analysis->transform(t);
}

void SinusoidalModel::interpolatePeak(const float x1, const float x2, const float x3,
                     const float y1, const float y2, const float y3, float &pX, float &pY){
    //adapted from http://stackoverflow.com/a/717791
    float denom = (x1 - x2) * (x1 - x3) * (x2 - x3);
    float a     = (x3 * (y2 - y1) + x2 * (y1 - y3) + x1 * (y3 - y2)) / denom;
    float b     = (x3*x3 * (y1 - y2) + x2*x2 * (y3 - y1) + x1*x1 * (y2 - y3)) / denom;
    float c     = (x2 * x3 * (x2 - x3) * y1 + x3 * x1 * (x3 - x1) * y2 + x1 * x2 * (x1 - x2) * y3) / denom;
    
    pX = -b / (2 * a);
    pY = c - b * b / (4 * a);
}

void SinusoidalModel::breakpoint(){
    float * magnitudes = &analysis->getMagnitudes();
    float * phases = &analysis->getPhases();
    float * frequencies = &analysis->getFrequencies();
    float mag, magL, magR, phs, phsL, phsR, frq, frqL, frqR,
    peakMag, peakPhs, peakFrq, lookupFrq;//,maxMag = analysis->getMaxMag();
    bool matched;
    int deadIdx;
    memset(matches, false, sizeof(bool) * maxTracks);//these flags are for tracks
    for(int i = 1; i < maxTracks - 1; ++i){//loop over all frq bins
        trackMagThreshold = 1.0 / log(frequencies[i]); //adjust thresholds according to frequency range
        trackFrqThreshold = log(frequencies[i]);
        magL = magnitudes[i-1];
        mag = magnitudes[i];
        magR = magnitudes[i+1];
        if(mag > trackMagThreshold && magL < mag && mag > magR){
            //at local max
            frqL = frequencies[i-1];
            frq = frequencies[i];
            frqR = frequencies[i+1];
            phsL = phases[i-1];
            phs = phases[i];
            phsR = phases[i+1];
            
            //quadratically interpolate mag/frq
            interpolatePeak(magL, mag, magR, frqL, frq, frqR, peakMag, peakFrq);
            //linearly interpolate phase based on adjusted peak?
            //should maybe just use phase diff from last window to further refine frq estimate
            peakPhs = (peakFrq == frq)?phs:(peakFrq < frq)? (mag - magL)/(frq - frqL) * (peakFrq - frqL):
            (magR - mag)/(frqR - frq) * (frqR - peakFrq);
            //std::cout << "Peak detected. Frq: " << peakFrq << " Mag: " << peakMag << " Phs: " << peakPhs << std::endl;
            matched = false;//flag for matching this peak to a track
            deadIdx = -1;
            for(int j = 0; j < maxTracks; ++j){//attempt to match peak to existing track
                if(!tracks[j].isDead()){//only attempt to match to living or limbo tracks
                    if(!matches[j]){//skip this track if another peak has already matched to it
                        lookupFrq = tracks[j].frq;
                        if(peakFrq >= lookupFrq - trackFrqThreshold && peakFrq <= lookupFrq + trackFrqThreshold){
                            matched = true;//peak is within frq threshold of this track
                            matches[j] = true;
                            tracks[j].update(true, sqrt(peakMag), peakFrq, peakPhs);
                            //std::cout << "Track " << j << " matched. Frq: " << peakFrq << " Mag: " << peakMag << " Phs: " << peakPhs << std::endl;
                            //std::cout << "Track " << j << " matched at frq " << peakFrq << ". Age: " << tracks[j].aliveFrames << std::endl;
                            break;
                        }
                    }
                }
                else{//store this for a slight speed boost..
                    deadIdx = j;
                }
            }
            if(!matched){//create new track
                if(deadIdx == -1){//did not encounter a dead track on previous pass, must search for one
                    for(int j = 0; j < maxTracks; ++j){//might be a good place to use binary search is this proves costly
                        if(tracks[j].isDead()){
                            deadIdx = j;
                        }
                    }
                }
                if(deadIdx == -1){//edge case, all tracks in use. randomly steal one
                    std::default_random_engine generator;
                    std::uniform_int_distribution<int> distribution(0, maxTracks - 1);
                    deadIdx = distribution(generator);
                    tracks[deadIdx].status = Track::STATUS::DEAD;
                    activeTracks--;//book keeping, just killed a track to make room for a new one
                }
                //assert(tracks[deadIdx].status == Track::STATUS::DEAD);
                tracks[deadIdx].init(this, sqrt(peakMag), peakFrq, peakPhs);
            }
        }
    }
    for(int i = 0; i < maxTracks; ++i){
        if(tracks[i].isActive()){//do another pass to check to update active tracks that may have gone stale matches[i]){//
                                 //assert(tracks[i].status != Track::STATUS::DEAD && tracks[i].status != Track::STATUS::BIRTH);
            tracks[i].update(false);
        }
    }
    //std::cout << "Synthesizing " << activeTracks << " peaks" << std::endl;
}






