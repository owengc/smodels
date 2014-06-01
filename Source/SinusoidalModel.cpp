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
#define ONEOVERTWENTY 0.05

bool matchSort(const TrackMatch &a, const TrackMatch &b){
	float frqDiffA = a.frqDiff, frqDiffB = b.frqDiff;
	if(frqDiffA == frqDiffB){//if tied, sort by overall distanceSq
		return a.distSq < b.distSq;
	}
	return frqDiffA < frqDiffB;//otherwise sort by frqDiff
}

float SinusoidalModel::getCurve(const ThresholdFunction tf, const float x){
	switch(tf){
		case ThresholdFunction::oneOverX:
			return 1.0 / x;
			break;
		case ThresholdFunction::logX:
			return logf(x + CRUMB);
			break;
		case ThresholdFunction::logXOverX:
			return logf(x + CRUMB) / x;
			break;
		case ThresholdFunction::logXSqOverX:
			return powf(logf(x + CRUMB), 2) / x;
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
    peakThresholds = new float[maxTracks]{0.0};
	detected = new TrackMatch[maxTracks];
    matches = new bool[maxTracks]{false};
	candidates = new TrackMatch[maxTracks];
//	matchMatrix.resize(maxTracks * MATCHMATRIXDEPTH); //treating flat vector as matrix
//	candidates.resize(MATCHMATRIXDEPTH); //used for sorting potential matches
	
	samplingRateOverSize = analysis->getSamplingRateOverSize();
	magThreshFnc = ThresholdFunction::logX;
	freqThreshFnc = ThresholdFunction::logXOverX;
	magThresholdFactor = 2.0; //[?, ?]
	frqThresholdFactor = 50.0; //[?, ?]
	peakThresholdFactor = 5.0;
	std::cout << "Magnitude/Frequency Thresholds: " << std::endl;
    float * frequencies = &analysis->getFrequencies();
    for(int i = 0; i < maxTracks; ++i){
        oscillators[i].init(wavetable, sr);
        //adjust thresholds according to frequency range
        magnitudeThresholds[i] = 20.0 * log10f(1.0 / (magThresholdFactor * frequencies[i]) + CRUMB); 
        frequencyThresholds[i] = logf(frequencies[maxTracks - i]) + frqThresholdFactor * logf(frequencies[i] + CRUMB) / (frequencies[i] + CRUMB);
//        peakThresholds[i] = (1.0 / (peakThresholdFactor * logf(frequencies[maxTracks - i])) + CRUMB);
		peakThresholds[i] = (1.0 / (peakThresholdFactor * logf(frequencies[i])) + CRUMB);
		std::cout << "Bin " << i << " frq: " << frequencies[i] << std::endl <<
		"M: " << magnitudeThresholds[i] << ", F: " << frequencyThresholds[i] << ", P: " << peakThresholds[i] << std::endl;
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
    delete[] peakThresholds;
	delete[] candidates;
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
	int numChecked = 0;
	if(activeTracks == 0){
		std::cout << "no active tracks" << std::endl;
		return out;
	}
    for(int i = 0; i < maxTracks; ++i){//get output from active tracks
        if(tracks[i].active){
            out += oscillators[i].next() * logf(tracks[i].aliveFrames) * fadeFactor;
			numChecked++;
			if(numChecked == activeTracks){
				break;
			}
        }
    }

	out /= sqrt((float)activeTracks); /**/ //
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
    float * frequencies = &analysis->getFrequencies();
    float * phases = &analysis->getPhases();
    float mag, magL, magLDiff, magR, magRDiff, phs, phsL, phsR, frq, frqL, frqR,
    peakAmp, peakMag, peakPhs, peakFrq, lookupAmp, lookupFrq, lookupPhs,
	frqDiff, magThreshold, frqThreshold, peakThreshold, ampScale = analysis->getNormFactor();
    int i, j, k, numNewTracks = 0, baseIdx, tailIdx, deadIdx, maxTracksMinusOne = maxTracks - 1;
	bool matched;
    memset(matches, false, sizeof(bool) * maxTracks);
    for(i = 1; i < maxTracksMinusOne; ++i){//loop over frq bins
		detected[i].reset();
		magThreshold = magnitudeThresholds[i];//pick threshold according to frequency range
		magL = magnitudes[i-1];
        mag = magnitudes[i];
        magR = magnitudes[i+1];
        if(mag > magThreshold && magL < mag && mag > magR){//at local max
			peakThreshold = peakThresholds[i];
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
			magLDiff = powf(10.0, (peakMag - magL) * ONEOVERTWENTY) - 1.0;//percentage difference from peak
			magRDiff = powf(10.0, (peakMag - magR) * ONEOVERTWENTY) - 1.0;//percentage difference from peak
			if(magLDiff > peakThreshold || magRDiff > peakThreshold){
//				std::cout << "magLDiff: " << magLDiff << ", magRDiff: " << magRDiff << std::endl;
	            peakAmp = powf(10.0, peakMag * ONEOVERTWENTY) * ampScale;
        	    //peakPhs = phs;
        	    //std::cout << "Peak " << i << " detected. Frq: " << peakFrq << " Amp: "<< peakAmp << " Mag: " << peakMag << " Phs: " << peakPhs << std::endl;
				detected[i].init(i, true, peakAmp, peakFrq, peakPhs);
				numNewTracks++;
			}
        }
    }
	//attempt to match detected peaks to existing tracks
	for(j = 0; j < maxTracks; ++j){//looping over tracks
		if(tracks[j].status != Track::STATUS::DEAD){//only attempt to match to living or limbo tracks
			candidates[j].reset();
			lookupAmp = tracks[j].amp;
			lookupFrq = tracks[j].frq;
			lookupPhs = tracks[j].phs;
//			baseIdx = j * MATCHMATRIXDEPTH;
//			tailIdx = MATCHMATRIXDEPTH - 1;
//			for(k = 0; k < MATCHMATRIXDEPTH; ++k){
//				candidates[k].reset();
//			}
//			float minDiff = MAXFLOAT, minThresh = MAXFLOAT; //debugging only
//			int minIdx = -1; //debugging only
//			matched = false;
			for(i = 1; i < maxTracksMinusOne; ++i){//looping over detections
				if(detected[i].detected){
					frqThreshold = frequencyThresholds[i];//this index still has relevant frq range info associated with it
					frqDiff = fabs(lookupFrq - detected[i].frq);//need abs for comparisons
					if(frqDiff < frqThreshold){//potential match here
						//test against 'worst best match'
//						if(frqDiff < candidates[tailIdx].frqDiff){//replace 'worst best' with this one
						detected[i].setDistanceSq(lookupAmp, lookupFrq, lookupPhs);
						if(matchSort(detected[i], candidates[j])){
							candidates[j] = detected[i];
//							candidates[tailIdx].setDistanceSq(lookupAmp, lookupFrq, lookupPhs);
//							std::sort(candidates.begin(), candidates.end(), matchSort);
//							matched = true;
//							if(frqDiff < minDiff){
//								matched = true;
//								minDiff = frqDiff;
//								minThresh = frqThreshold;
//								minIdx = i;
//								std::cout << "matrix idx: " << baseIdx << ", matrix val: " << candidates[tailIdx].idx << ", detected val: " << detected[i].idx << std::endl;
//							}
						}
						
					}
				}
			}
//			if(matched){
//				std::copy(candidates.begin(), candidates.end(), matchMatrix.begin() + baseIdx);
//				std::cout << "track " << j << ", baseIdx: " << baseIdx << ", minIdx: " << minIdx << ", minfrqDiff: " << minDiff << ", threshold: " << minThresh << std::endl;
//				std::cout << "attempting to match track with frq " << lookupFrq << ". candidates: " << std::endl;
//				for(k = 0; k < MATCHMATRIXDEPTH; ++k){
//					std:: cout << "idx " << matchMatrix[baseIdx + k].idx << " frq: " << matchMatrix[baseIdx + k].frq << ", ";
//				}
//				std::cout << std::endl << std::endl;
//			}
		}
	}
//	std::cout << "num new before: " << numNewTracks << std::endl;
	//now that all potential matches have been found for each track, let's assign the detected peaks
	
	longestTrack = 1;
	for(j = 0; j < maxTracks; ++j){//looping over tracks
		if(tracks[j].status != Track::STATUS::DEAD){//only attempt to match to living or limbo tracks
			baseIdx = j * MATCHMATRIXDEPTH;
			matched = false;
//			i = -1;
//			for(k = 0; k < MATCHMATRIXDEPTH - 1; ++k){
//				i = matchMatrix[baseIdx + k].idx;
//				if(i > 0 && detected[i].detected && !detected[i].assigned){
//					std::cout << "using " << k << "th candidate" << std::endl;
//					matched = true;
//					break;
//				}
//			}
			i = candidates[j].idx;
			if(i > 0 && detected[i].detected && !detected[i].assigned){
				matched = true;
			}
			if(matched){
				peakAmp = detected[i].amp;
				peakFrq = detected[i].frq;
				peakPhs = detected[i].phs;
				tracks[j].update(true, peakAmp, peakFrq, peakPhs);
				oscillators[j].update(peakAmp, peakFrq, peakPhs, hopSize);
				detected[i].assigned = true;
				matches[j] = true;
				numNewTracks--;
			}
				//std::cout << "Track " << j << " matched. Frq: " << peakFrq << " Mag: " << peakMag << " Phs: " << peakPhs << std::endl;
				//std::cout << "Track " << j << " matched at frq " << peakFrq << ". Age: " << tracks[j].aliveFrames << std::endl;
		}
	}
	fadeFactor = 1.0 / (1.0 + logf(longestTrack));
//	std::cout << "num new after: " << numNewTracks << std::endl;
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






