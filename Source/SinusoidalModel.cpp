/*
  ==============================================================================

    SinusoidalModel.cpp
    Created: 11 Mar 2014 4:50:42pm
    Author:  Owen Campbell

  ==============================================================================
*/

#include "SinusoidalModel.h"
#include "Track.h"

SinusoidalModel::SinusoidalModel(const Analysis::WINDOW w, const int ws, const int hf, const float sr, const bool p,
                                 Wavetable<float>::WAVEFORM wf, const int wts){
    windowSize = ws;
    //delete wavetable;
    wavetable = new Wavetable<float>(wf, wts);
    
    analysis = new Analysis(w, ws, hf, sr, p);
    maxTracks = analysis->getNumBins();
    hopSize = analysis->getAppetite();
    
    //delete[] tracks;
    tracks = new Track[maxTracks];
    //delete[] oscillators;
    oscillators = new Oscillator<float>[maxTracks];
    //delete[] magnitudeThresholds;
    magnitudeThresholds = new float[maxTracks]{0.0};
    //delete[] frequencyThresholds;
    frequencyThresholds = new float[maxTracks]{0.0};
    //delete[] matches;
    matches = new bool[maxTracks]{false};
    active = new bool[maxTracks]{false};
    
    
    float * frequencies = &analysis->getFrequencies();
    for(int i = 0; i < maxTracks; ++i){
        oscillators[i].init(wavetable, sr);
        //adjust thresholds according to frequency range
        magnitudeThresholds[i] = 1.0 / frequencies[i];
        frequencyThresholds[i] = log(frequencies[i]);
    }
    /*analysis = nullptr;
    wavetable = nullptr;
    tracks = nullptr;
    oscillators = nullptr;
    matches = nullptr;
    magnitudeThresholds = nullptr;
    frequencyThresholds = nullptr;*/
}
SinusoidalModel::~SinusoidalModel(){
    delete analysis;
    delete wavetable;
    delete[] tracks;
    delete[] oscillators;
    delete[] matches;
    delete[] magnitudeThresholds;
    delete[] frequencyThresholds;
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
void SinusoidalModel::init(){
    
    activeTracks = 0;
    //hard coding these for now
    trackBirth = 0;
    trackDeath = 10;
    
    srand(time(0));
}

bool SinusoidalModel::operator() (const float sample){//use this to write samples to the input buffer
    return analysis->operator()(sample);
}

float SinusoidalModel::operator() (void){//use this to read samples from the output buffer
    float out = 0.0;
	if(activeTracks == 0){
		return out;
	}
	int numChecked = 0;
    for(int i = 0; i < maxTracks; ++i){//get output from active tracks
        if(tracks[i].active){
            out += oscillators[i].next();
			numChecked++;
			if(numChecked == activeTracks){
				break;
			}
        }
    }
    return out / activeTracks;
}

void SinusoidalModel::transform(const Analysis::TRANSFORM t){
    analysis->transform(t);
}

void SinusoidalModel::interpolatePeak(const float x1, const float x2, const float x3,
                     const float y1, const float y2, const float y3, float &pX, float &pY){
    //adapted from http://stackoverflow.com/a/717791
    float x1_Minus_x2 = x1 - x2, x2_Minus_x3 = x2 - x3, oneOverDenom = 1.0 / (x1_Minus_x2) * (x1 - x3) * (x2_Minus_x3);
    const float a     = (x3 * (y2 - y1) + x2 * (y1 - y3) + x1 * (y3 - y2)) * oneOverDenom;
    const float b     = (x3*x3 * (y1 - y2) + x2*x2 * (y3 - y1) + x1*x1 * (y2 - y3)) * oneOverDenom;
    const float c     = (x2 * x3 * (x2_Minus_x3) * y1 + x3 * x1 * (x3 - x1) * y2 + x1 * x2 * (x1_Minus_x2) * y3) * oneOverDenom;
    
    pX = -b / (2 * a);
    pY = c - b * b / (4 * a);
}//TODO: make this faster

void SinusoidalModel::breakpoint(){
    hopSize = analysis->getAppetite();
    float * magnitudes = &analysis->getMagnitudes();
    float * phases = &analysis->getPhases();
    float * frequencies = &analysis->getFrequencies();
    float mag, magL, magR, phs, phsL, phsR, frq, frqL, frqR,
    peakAmp, peakMag, peakPhs, peakFrq, lookupFrq;//,maxMag = analysis->getMaxMag();
    bool matched;
    int deadIdx;
    memset(matches, false, sizeof(bool) * maxTracks);//these flags are for tracks
    for(int i = 1; i < maxTracks - 1; ++i){//loop over all frq bins
        trackMagThreshold = magnitudeThresholds[i]; //adjust thresholds according to frequency range
        trackFrqThreshold = frequencyThresholds[i];
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
            peakMag = sqrt(peakMag);
            peakAmp = (peakMag > 1.0)?1.0:(peakMag < 0.0 || isnan(peakMag))?0.0:peakMag;//TODO: figure out why NaNs showed up
            //
            if(peakFrq < 0.0){//TODO: figure out why negatives showed up
                peakFrq = fabs(peakFrq);
            }
            peakPhs = phs;
            //std::cout << "Peak detected. Frq: " << peakFrq << " Mag: " << peakMag << " Phs: " << peakPhs << std::endl;
            matched = false;//flag for matching this peak to a track
            deadIdx = -1;
            for(int j = 0; j < maxTracks; ++j){//attempt to match peak to existing track
                if(tracks[j].status != Track::STATUS::DEAD){//only attempt to match to living or limbo tracks
                    
                    //TODO: if already matched, see if this one is closer than previously matched track
                    //      test matches using weighted distance of both frq and mag
                    
                    if(!matches[j]){//skip this track if another peak has already matched to it
                        lookupFrq = tracks[j].frq;
                        if(peakFrq >= lookupFrq - trackFrqThreshold && peakFrq <= lookupFrq + trackFrqThreshold){
                            matched = true;//peak is within frq threshold of this track
                            matches[j] = true;
                            tracks[j].update(true, peakAmp, peakFrq, peakPhs);
                            oscillators[j].update(peakAmp, peakFrq, peakPhs, analysis->getAppetite());
                            //std::cout << "Track " << j << " matched. Frq: " << peakFrq << " Mag: " << peakMag << " Phs: " << peakPhs << std::endl;
                            //std::cout << "Track " << j << " matched at frq " << peakFrq << ". Age: " << tracks[j].aliveFrames << std::endl;
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
                        if(tracks[j].status == Track::STATUS::DEAD){
                            deadIdx = j;
                        }
                    }
					if(deadIdx == -1){//edge case, all tracks in use. randomly steal one
						deadIdx = (rand() % maxTracks) + 1;
						tracks[deadIdx].status = Track::STATUS::DEAD;
					}
                }
                //std::cout << "amp: " << peakAmp << ", frq: " << peakFrq << ", phs: " << peakPhs << std::endl;
                tracks[deadIdx].init(this, peakAmp, peakFrq, peakPhs);
                oscillators[deadIdx].start(peakAmp, peakFrq, peakPhs);
            }
        }
    }
    activeTracks = 0;
    for(int i = 0; i < maxTracks; ++i){
        if(tracks[i].active){//do another pass to update active tracks that may have gone stale
            activeTracks++;
            if(!matches[i]){
                tracks[i].update(false);
            }
        }
    }
    //std::cout << "Synthesizing " << activeTracks << " of " << maxTracks << " possible tracks" << std::endl;
}






