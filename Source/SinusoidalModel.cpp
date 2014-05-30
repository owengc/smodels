/*
  ==============================================================================

    SinusoidalModel.cpp
    Created: 11 Mar 2014 4:50:42pm
    Author:  Owen Campbell

  ==============================================================================
*/

#include "SinusoidalModel.h"
#include "Track.h"
#define CRUMB 0.0000001

float SinusoidalModel::getCurve(const ThresholdFunction tf, const float x){
	switch(tf){
		case ThresholdFunction::oneOverX:
			return 1.0 / x;
			break;
		case ThresholdFunction::logX:
			return log(x + CRUMB);
			break;
		case ThresholdFunction::logXOverX:
			return log(x + CRUMB) / x;
			break;
		case ThresholdFunction::logXSqOverX:
			return pow(log(x + CRUMB), 2) / x;
			break;
		default:
			return x;
	}
}

SinusoidalModel::SinusoidalModel(const Analysis::WINDOW w, const int ws, const int hf, const float sr, const bool p,
                                 Wavetable<float>::WAVEFORM wf, const int wts){
    windowSize = ws;
    wavetable = new Wavetable<float>(wf, wts);
    analysis = new Analysis(w, ws, hf, sr, p);
    maxTracks = analysis->getNumBins();
    hopSize = analysis->getAppetite();
    
    tracks = new Track[maxTracks];
    oscillators = new Oscillator<float>[maxTracks];
    magnitudeThresholds = new float[maxTracks]{0.0};
    frequencyThresholds = new float[maxTracks]{0.0};
    matches = new bool[maxTracks]{false};
    active = new bool[maxTracks]{false};
    
	
	samplingRateOverSize = analysis->getSamplingRateOverSize();
	magThreshFnc = ThresholdFunction::logX;
	freqThreshFnc = ThresholdFunction::logXOverX;
	magThresholdFactor = 2.0; //[?, ?]
	frqThresholdFactor = 50.0; //[?, ?]
	std::cout << "Magnitude/Frequency Thresholds: " << std::endl;
    float * frequencies = &analysis->getFrequencies();
    for(int i = 0; i < maxTracks; ++i){
        oscillators[i].init(wavetable, sr);
        //adjust thresholds according to frequency range
        magnitudeThresholds[i] = 20.0 * log10f(1.0 / (magThresholdFactor * frequencies[i]) + CRUMB); 
        frequencyThresholds[i] = frqThresholdFactor * log(frequencies[i] + CRUMB) / (frequencies[i] + CRUMB);
		std::cout << "Bin " << i << " frq: " << frequencies[i] << std::endl <<
		"M: " << magnitudeThresholds[i] << ", F: " << frequencyThresholds[i] << std::endl;
    }
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
        case Analysis::PARAMETER::AMP:
            return &analysis->getAmplitudes();
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
float SinusoidalModel::getAmpNormFactor() const{
	return analysis->getAmplitudeNormalizationFactor();
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
    return analysis->operator()(sample) ;
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
    return out  / (float)activeTracks;//analysis->getAmplitudeNormalizationFactor();
}

void SinusoidalModel::transform(const Analysis::TRANSFORM t){
    analysis->transform(t);
}

void SinusoidalModel::interpolatePeak(const int mIdx, const float ml, const float m, const float mr, float &pm, float &pf){
	float p, d;
	d = (ml - mr);
	p = 0.5 * d / (ml + mr - 2.0 * m);
	pm = m - 0.25 * d * p;
	pf = (mIdx + p) * samplingRateOverSize;
}

void SinusoidalModel::breakpoint(){
    hopSize = analysis->getAppetite();
    float * magnitudes = &analysis->getMagnitudes();
    float * phases = &analysis->getPhases();
    float * frequencies = &analysis->getFrequencies();
    float mag, magL, magR, phs, phsL, phsR, frq, frqL, frqR,
    peakAmp, peakMag, peakPhs, peakFrq, lookupFrq, magThreshold, frqThreshold;//,maxMag = analysis->getMaxMag();
    bool matched;
    int deadIdx;
    memset(matches, false, sizeof(bool) * maxTracks);//these flags are for tracks
    for(int i = 1; i < maxTracks - 1; ++i){//loop over all frq bins
        magThreshold = magnitudeThresholds[i]; //adjust thresholds according to frequency range
        frqThreshold = frequencyThresholds[i];
        magL = magnitudes[i-1];
        mag = magnitudes[i];
        magR = magnitudes[i+1];
        if(mag > magThreshold && magL < mag && mag > magR){
            //at local max
            frqL = frequencies[i-1];
            frq = frequencies[i];
            frqR = frequencies[i+1];
            phs = phases[i];
            //quadratically interpolate peak
            //std::cout << "interpolating mags {" << magL << ", " << mag << ", " << magR << "} and frqs {" <<
            //frqL << ", " << frq << ", " << frqR << "}" << std::endl;
            //interpolatePeak(magL, mag, magR, frqL, frq, frqR, peakMag, peakFrq);
            interpolatePeak(i, magL, mag, magR, peakMag, peakFrq);
			//std::cout << "mag comparison: " << magL << ", " << mag << ", " << magR << std::endl;
			//std::cout << "interped mag: " << peakMag << std::endl;
			//std::cout << "frq comparison: " << frqL << ", " << frq << ", " << frqR << std::endl;
			//std::cout << "interped frq: " << peakFrq << std::endl;
            peakAmp = pow(10, peakMag / 20.0);//sqrt(peakMag);
			if(peakAmp > 1.0){
				//std::cout << "clipped amp: " << peakAmp << std::endl;
			}
			else{
				//std::cout << "amp: " << peakAmp << std::endl;
			}
            //peakAmp = (peakMag > 1.0)?1.0:(peakMag < 0.0 || isnan(peakMag))?0.0:peakMag;//TODO: figure out why NaNs showed up
            peakPhs = phs;
            //std::cout << "Peak detected. Frq: " << peakFrq << " Mag: " << peakMag << " Phs: " << peakPhs << std::endl;
            matched = false;//flag for matching this peak to a track
            deadIdx = -1;
			
            for(int j = 0; j < maxTracks; ++j){//attempt to match peak to existing track
                if(tracks[j].status != Track::STATUS::DEAD){//only attempt to match to living or limbo tracks
                    
                    //TODO: if already matched, see if this one is closer than previously matched track
                    //      test matches using weighted distance of both frq and mag
                    
                    //if(!matches[j]){//skip this track if another peak has already matched to it
                        lookupFrq = tracks[j].frq;
                        if(peakFrq >= lookupFrq - frqThreshold && peakFrq <= lookupFrq + frqThreshold){
                            matched = true;//peak is within frq threshold of this track
                            matches[j] = true;
                            tracks[j].update(true, peakAmp, peakFrq, peakPhs);
                            oscillators[j].update(peakAmp, peakFrq, peakPhs, analysis->getAppetite());
                            //std::cout << "Track " << j << " matched. Frq: " << peakFrq << " Mag: " << peakMag << " Phs: " << peakPhs << std::endl;
                            //std::cout << "Track " << j << " matched at frq " << peakFrq << ". Age: " << tracks[j].aliveFrames << std::endl;
                            break;
                        }
                    //}
                }
                else{
                    deadIdx = j;//store this for a slight speed boost later..
                }
            }
            if(!matched){//create new track
                if(deadIdx == -1){//did not encounter a dead track on previous pass, must search for one
                    for(int j = 0; j < maxTracks; ++j){//might be a good place to use binary search if this proves costly
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






