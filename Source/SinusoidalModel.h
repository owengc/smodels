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
class Track;

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
    bool * matches, active;
    float * magnitudeThresholds, * frequencyThresholds;
    int windowSize, hopSize, maxTracks, activeTracks, trackBirth, trackDeath;
    float magThresholdFactor, frqThresholdFactor;
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
    void interpolatePeak(const float x1, const float x2, const float x3,
                         const float y1, const float y2, const float y3, float &pX, float &pY);
    void breakpoint();
	int getNumActive(){ return activeTracks; };
};



#endif  // SINUSOIDALMODEL_H_INCLUDED
