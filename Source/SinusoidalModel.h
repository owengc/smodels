/*
  ==============================================================================

    SinusoidalModel.h
    Created: 11 Mar 2014 4:50:42pm
    Author:  Owen Campbell

  ==============================================================================
*/

#ifndef SINUSOIDALMODEL_H_INCLUDED
#define SINUSOIDALMODEL_H_INCLUDED

#include "Analysis.h"
#include "Oscillator.h"
#include "Noise.h"
#include <cassert>
#include <ctime>

#define MATCHMATRIXDEPTH 3

class Track;
class TrackMatch;
enum class ThresholdFunction{
	oneOverX,
	logX,
	logXOverX,
	logXSqOverX
};

class SinusoidalModel{
friend class Track;
private:
    Analysis * analysis;
    Track * tracks;
    Oscillator<float> * oscillators;
    Wavetable<float> * wavetable;
    bool * matches;
    float * magnitudeThresholds, * frequencyThresholds, * peakThresholds;
	TrackMatch * detected, * candidates;
	
    int windowSize, hopSize, maxTracks, activeTracks, trackBirth, trackDeath, longestTrack;
    float magThresholdFactor, frqThresholdFactor, peakThresholdFactor, samplingRateOverSize, fadeFactor;
	ThresholdFunction freqThreshFnc, magThreshFnc;
	
public:
    SinusoidalModel(const Analysis::WINDOW w, const int ws, const int hf, const float sr, const bool p,
                    Wavetable<float>::WAVEFORM wf, const int wts);
    ~SinusoidalModel();
    //getters
    float * getAnalysisResults(const Analysis::PARAMETER p) const;
	float getAmpNormFactor() const;

    //setters
    void setWaveform(Wavetable<float>::WAVEFORM wf);
    
    //business/helper functions
    void init();
	float getCurve(const ThresholdFunction tf, const float x);
    
    bool operator() (const float sample);//use this to write samples to the input buffer
    float operator() (void);//use this to read samples from the output buffer
    void transform(const Analysis::TRANSFORM t);
    void interpolatePeak(const int mIdx, const float ml, const float m, const float mr,
						 const float pL, const float p, const float pR, float &pm, float &pf, float &pp);
    void interpolatePeak(const int mIdx, const float ml, const float m, const float mr,
						 float &pm, float &pf);
	
    void breakpoint();
	int getNumActive(){ return activeTracks; };
};



#endif  // SINUSOIDALMODEL_H_INCLUDED
