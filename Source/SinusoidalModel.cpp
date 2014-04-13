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
void SinusoidalModel::updateOscillator(const int idx, Track * t){
    oscillators[idx].setFrequency(t->frq);
}


//business/helper functions
void SinusoidalModel::init(const Analysis::WINDOW w, const int ws, const float sr, const bool p,
          Wavetable<float>::WAVEFORM wf, const int wts){
    windowSize = ws;
    delete wavetable;
    wavetable = new Wavetable<float>(wf, wts);
    analysis->init(w, ws, sr, p);
    maxTracks = analysis->getNumBins();
    
    delete[] tracks;
    tracks = new Track[maxTracks];
    delete[] oscillators;
    oscillators = new Oscillator<float>[maxTracks];
    for(int i = 0; i < maxTracks; ++i){
        oscillators[i].init(wavetable, sr, 0.0, 1.0, 0.0);
    }
    delete[] matches;
    matches = new bool[maxTracks]{false};
    
    activeTracks = 0;
    //hard coding these for now
    trackBirth = 10;
    trackDeath = 5;
    
    srand(time(0));
}

bool SinusoidalModel::operator() (const float sample){//use this to write samples to the input buffer
    return analysis->operator()(sample);
}

float SinusoidalModel::operator() (void){//use this to read samples from the output buffer
    float out = 0.0;
    int numActive = 0, i =0;
    for(; i < maxTracks; ++i){//attempt to match peak to existing track
        if(tracks[i].isActive()){
            numActive++;
            //out += oscillators[i].next();
        }
    }
    return (numActive > 1)?out / numActive:out;
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
    peakAmp, peakMag, peakPhs, peakFrq, lookupFrq, fraction;//,maxMag = analysis->getMaxMag();
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
            
            //quadratically interpolate peak
            //std::cout << "interpolating mags {" << magL << ", " << mag << ", " << magR << "} and frqs {" <<
            //frqL << ", " << frq << ", " << frqR << "}" << std::endl;
            //interpolatePeak(magL, mag, magR, frqL, frq, frqR, peakMag, peakFrq);
            interpolatePeak(frqL, frq, frqR, magL, mag, magR, peakFrq, peakMag);
            peakAmp = sqrt(peakMag);
            if(peakFrq < 0.0){
                peakFrq = fabs(peakFrq);
            }
                        //should maybe just use phase diff from last window to further refine frq estimate
            /*if(peakFrq == frq){//linearly interpolate phase based on adjusted peak?

                peakPhs = phs;
            }
            else if(peakFrq < frq){
                fraction = (peakFrq - frqL) / (frq - frqL);
                peakPhs =
            }
            else{
                
            }//(magR - mag)/(frqR - frq) * (frqR - peakFrq);
            
            peakPhs = fabs(peakPhs);*/
            peakPhs = phs;
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
                            tracks[j].update(true, peakAmp, peakFrq, peakPhs);
                            //oscillators[j].update(peakAmp, peakFrq, peakPhs);
                            //std::cout << "Track " << j << " matched. Frq: " << peakFrq << " Mag: " << peakMag << " Phs: " << peakPhs << std::endl;
                            std::cout << "Track " << j << " matched at frq " << peakFrq << ". Age: " << tracks[j].aliveFrames << std::endl;
                            break;
                        }
                    }
                }
                else{
                    deadIdx = j;//store this for a slight speed boost later..
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
                    deadIdx = (rand() % maxTracks) + 1;
                    tracks[deadIdx].status = Track::STATUS::DEAD;
                    activeTracks--;//book keeping, just killed a track to make room for a new one
                }
                //assert(tracks[deadIdx].status == Track::STATUS::DEAD);
                //std::cout << "amp: " << peakAmp << ", frq: " << peakFrq << ", phs: " << peakPhs << std::endl;
                tracks[deadIdx].init(this, peakAmp, peakFrq, peakPhs);
                //oscillators[deadIdx].start(peakAmp, peakFrq, peakPhs);
            }
        }
    }
    for(int i = 0; i < maxTracks; ++i){
        if(tracks[i].isActive()){//do another pass to check to update active tracks that may have gone stale matches[i]){//
                                 //assert(tracks[i].status != Track::STATUS::DEAD && tracks[i].status != Track::STATUS::BIRTH);
            tracks[i].update(false);
        }
        else if(tracks[i].isDead()){
            //oscillators[i].stop();//quiet down the associated oscillator just in case
        }
    }
     std::cout << "Synthesizing " << activeTracks << " peaks" << std::endl;
}






