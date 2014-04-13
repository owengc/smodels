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

class SinusoidalModel{
friend class Track;
private:
    Analysis * analysis;
    Track * tracks;
    Oscillator<float> * oscillators;
    Wavetable<float> * wavetable;
    bool * matches;
    int windowSize, maxTracks, activeTracks, trackBirth, trackDeath;
    float trackMagThreshold, trackFrqThreshold;
public:
    SinusoidalModel();
    ~SinusoidalModel();
    //getters
    float * getAnalysisResults(const Analysis::PARAMETER p) const;
    //setters
    void setWaveform(Wavetable<float>::WAVEFORM wf);
    void updateOscillator(const int idx, const Track &t);
    
    //business/helper functions
    void init(const Analysis::WINDOW w, const int ws, const float sr, const bool p,
              Wavetable<float>::WAVEFORM wf, const int wts);
    
    bool operator() (const float sample);//use this to write samples to the input buffer
    float operator() (void);//use this to read samples from the output buffer
    void transform(const Analysis::TRANSFORM t);
    void interpolatePeak(const float x1, const float x2, const float x3,
                         const float y1, const float y2, const float y3, float &pX, float &pY);
    void breakpoint();
    void updateOscillator(const int idx, Track * t);
};



#endif  // SINUSOIDALMODEL_H_INCLUDED
