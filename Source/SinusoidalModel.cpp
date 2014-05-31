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

int matchSort(TrackMatch &a, TrackMatch &b){
	float frqDiffA = a.frqDiff, frqDiffB = b.frqDiff,
 	distSqA = a.distSq, distSqB = b.distSq;
	if(frqDiffA < frqDiffB){//first sort by frqDiff
		return -1;
	}
	else if(frqDiffA > frqDiffB){
		return 1;
	}
	else{//if tied, sort by overall distanceSq
		if(distSqA < distSqB){
			return -1;
		}
		else{
			return distSqA > distSqB;
		}
	}
}

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
	detected = new TrackMatch[maxTracks];
    matches = new bool[maxTracks]{false};
    active = new bool[maxTracks]{false};
	matchMatrix = new TrackMatch*[maxTracks];
	
	
	samplingRateOverSize = analysis->getSamplingRateOverSize();
	magThreshFnc = ThresholdFunction::logX;
	freqThreshFnc = ThresholdFunction::logXOverX;
	magThresholdFactor = 2.0; //[?, ?]
	frqThresholdFactor = 50.0; //[?, ?]
	std::cout << "Magnitude/Frequency Thresholds: " << std::endl;
    float * frequencies = &analysis->getFrequencies();
    for(int i = 0; i < maxTracks; ++i){
		matchMatrix[i] = new TrackMatch[MATCHMATRIXDEPTH];
        oscillators[i].init(wavetable, sr);
        //adjust thresholds according to frequency range
        magnitudeThresholds[i] = 20.0 * log10f(1.0 / (magThresholdFactor * frequencies[i]) + CRUMB); 
        frequencyThresholds[i] = log(frequencies[maxTracks - i]) + frqThresholdFactor * log(frequencies[i] + CRUMB) / (frequencies[i] + CRUMB);
		std::cout << "Bin " << i << " frq: " << frequencies[i] << std::endl <<
		"M: " << magnitudeThresholds[i] << ", F: " << frequencyThresholds[i] << std::endl;
    }
}


SinusoidalModel::~SinusoidalModel(){
    delete analysis;
    delete wavetable;
    delete[] tracks;
    delete[] oscillators;
	delete[] detected;
    delete[] matches;
    delete[] magnitudeThresholds;
    delete[] frequencyThresholds;
	for(int i = 0; i < maxTracks; ++i){
		delete [] matchMatrix[i];
	}
	delete [] matchMatrix;
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
	return analysis->getNormFactor();
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

float SinusoidalModel::operator() (void){//use this to read samples from the oscillators
    float out = 0.0;
	if(activeTracks == 0){
		std::cout << "no active tracks" << std::endl;
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

	//out /= sqrt((float)activeTracks); /**/ //
										   //out /= analysis->getAmplitudeNormalizationFactor();
	//std::cout << activeTracks << " active tracks, output amp: " << out << std::endl;
	//std::cout << "numActive: " << activeTracks << std::endl;
    return out;// / analysis->getAmplitudeNormalizationFactor();//(float)activeTracks;
}

void SinusoidalModel::transform(const Analysis::TRANSFORM t){
    analysis->transform(t);
}

void SinusoidalModel::interpolatePeak(const int pIdx, const float ml, const float m, const float mr,
									const float pL, const float p, const float pR, float &pm, float &pf, float &pp){
	float idxOffset, diff, idx, frac;
	diff = (ml - mr);
	idxOffset = 0.5 * diff / (ml + mr - 2.0 * m);
	pm = m - 0.25 * diff * idxOffset;
	idx = pIdx + idxOffset;
	pf = idx * samplingRateOverSize;
	if(idx <= pIdx){
		frac = idx - pIdx + 1;
		pp = (1.0 - frac) * pL + frac * p;
	}
	else{
		frac = idx - pIdx;
		pp = frac * p + (1.0 - frac) * pR;
	}
}

void SinusoidalModel::interpolatePeak(const int pIdx, const float ml, const float m, const float mr,
									  float &pm, float &pf){
	float idxOffset, diff;
	diff = (ml - mr);
	idxOffset = 0.5 * diff / (ml + mr - 2.0 * m);
	pm = m - 0.25 * diff * idxOffset;
	pf = (pIdx + idxOffset) * samplingRateOverSize;
}

void SinusoidalModel::breakpoint(){
    hopSize = analysis->getAppetite();
    float * magnitudes = &analysis->getMagnitudes();
    float * phases = &analysis->getPhases();
    float * frequencies = &analysis->getFrequencies();
    float mag, magL, magR, phs, phsL, phsR, frq, frqL, frqR,
    peakAmp, peakMag, peakPhs, peakFrq, lookupFrq,
	frqDiff, ampDiff, phsDiff, magThreshold, frqThreshold, ampScale = analysis->getDenormFactor();;
    int i, j, k, numNewTracks = 0, deadIdx, maxTracksMinusOne = maxTracks - 1;
	bool matched;
	TrackMatch * potentialMatch, * competingMatch, * bestMatch;
    memset(matches, false, sizeof(bool) * maxTracks);
    for(i = 1; i < maxTracksMinusOne; ++i){//loop over frq bins
		detected[i].reset();
		magThreshold = magnitudeThresholds[i];//pick threshold according to frequency range
        magL = magnitudes[i-1];
        mag = magnitudes[i];
        magR = magnitudes[i+1];
        if(mag > magThreshold && magL < mag && mag > magR){
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
            //interpolatePeak(i, magL, mag, magR, peakMag, peakFrq);
			interpolatePeak(i, magL, mag, magR, phsL, phs, phsR, peakMag, peakFrq, peakPhs);
			//std::cout << "mag comparison: " << magL << ", " << mag << ", " << magR << std::endl;
			//std::cout << "interped mag: " << peakMag << std::endl;
			//std::cout << "frq comparison: " << frqL << ", " << frq << ", " << frqR << std::endl;
			//std::cout << "interped frq: " << peakFrq << std::endl;
			//std::cout << "phs comparison: " << phsL << ", " << phs << ", " << phsR << std::endl;
			//std::cout << "interped phs: " << peakPhs << std::endl;
            peakAmp = pow(10, peakMag / 20.0);
            //peakPhs = phs;
            //std::cout << "Peak detected. Frq: " << peakFrq << " Amp: "<< peakAmp << " Mag: " << peakMag << " Phs: " << peakPhs << std::endl;
			detected[i].idx = i;
			detected[i].detected = true;
			detected[i].amp = peakAmp;
			detected[i].frq = peakFrq;
			detected[i].phs = peakPhs;
			numNewTracks++;
        }
    }
	//attempt to match detected peaks to existing tracks
	for(j = 0; j < maxTracks; ++j){//looping over tracks
		if(tracks[j].status != Track::STATUS::DEAD){//only attempt to match to living or limbo tracks
			lookupFrq = tracks[j].frq;
			for(k = 0; k < MATCHMATRIXDEPTH; ++k){
				matchMatrix[j][k].reset();
			}
			//note: below I'm purposefully using the current value of k following this loop (MATCHMATRIXDEPTH -1)
			for(i = 1; i < maxTracksMinusOne; ++i){//looping over detections
				if(detected[i].detected){
					frqThreshold = frequencyThresholds[i];
					frqDiff = abs(lookupFrq - detected[i].frq);
					if(frqDiff < frqThreshold * 10.0){//potential match here
						potentialMatch = &detected[i];
						//treat matchMatrix vectors as priority queues
						competingMatch = &matchMatrix[j][k];//test against 'worst best' match
						if(frqDiff < competingMatch->frqDiff){//replace 'worst best' with this one
							potentialMatch->setDistanceSq(tracks[j].amp, lookupFrq, tracks[j].phs);
							matchMatrix[j][k] = *potentialMatch;
							std::sort(matchMatrix[j], matchMatrix[j] + MATCHMATRIXDEPTH, matchSort);
						}
					}
				}
			}
		}
	}
	std::cout << "num new before: " << numNewTracks << std::endl;
	//now that all potential matches have been found for each track, let's assign the detected peaks
	for(j = 0; j < maxTracks; ++j){//looping over tracks
		if(tracks[j].status != Track::STATUS::DEAD){//only attempt to match to living or limbo tracks
			matched = false;
			for(k = 0; k < MATCHMATRIXDEPTH - 1; ++k){
				bestMatch = &detected[matchMatrix[j][k].idx];
				if(!bestMatch->assigned){
					matched = true;
					break;
				}
			}
			if(matched){
				peakAmp = bestMatch->amp;
				peakFrq = bestMatch->frq;
				peakPhs = bestMatch->phs;
				tracks[j].update(true, peakAmp, peakFrq, peakPhs);
				oscillators[j].update(peakAmp, peakFrq, peakPhs, hopSize);
				bestMatch->assigned = true;
				matches[j] = true;
				numNewTracks--;
			}
				//std::cout << "Track " << j << " matched. Frq: " << peakFrq << " Mag: " << peakMag << " Phs: " << peakPhs << std::endl;
				//std::cout << "Track " << j << " matched at frq " << peakFrq << ". Age: " << tracks[j].aliveFrames << std::endl;
		}
	}
	std::cout << "num new after: " << numNewTracks << std::endl;
	//check if we need to start new tracks for remaining peaks
	for(i = 1; i < maxTracksMinusOne && numNewTracks > 0; ++i){//looping over detections
		if(detected[i].detected && !detected[i].assigned){//find a dead track idx and start a new track
			deadIdx = -1;
			for(int j = 0; j < maxTracks; ++j){//looping over tracks
				if(tracks[j].status == Track::STATUS::DEAD){
					deadIdx = j;
				}
			}
			if(deadIdx == -1){//edge case, all tracks in use. randomly steal one
				deadIdx = (rand() % maxTracksMinusOne);
				//std::cout << "stealing track " << deadIdx << " right meow" << std::endl;
				tracks[deadIdx].status = Track::STATUS::DEAD;
			}
			peakAmp = detected[i].amp;
			peakFrq = detected[i].frq;
			peakPhs = detected[i].phs;
			//std::cout << "amp: " << peakAmp << ", frq: " << peakFrq << ", phs: " << peakPhs << std::endl;
			tracks[deadIdx].init(this, peakAmp, peakFrq, peakPhs);
			oscillators[deadIdx].start(peakAmp, peakFrq, peakPhs);
			numNewTracks--;
		}
	}
    activeTracks = 0;
    for(j = 0; j < maxTracks; ++j){//looping over tracks
        if(tracks[j].active){//do another pass to update active tracks that may have gone stale
            activeTracks++;
            if(!matches[j]){
                tracks[j].update(false);
            }
        }
    }
    //std::cout << "Synthesizing " << activeTracks << " of " << maxTracks << " possible tracks" << std::endl;
}






