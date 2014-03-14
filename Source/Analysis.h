/*
  ==============================================================================

    Analysis.h
    Created: 1 Mar 2014 10:17:35pm
    Author:  Owen Campbell

  ==============================================================================
*/

#ifndef ANALYSIS_H_INCLUDED
#define ANALYSIS_H_INCLUDED

#include <algorithm>
#include <cmath>
#include <cassert>
#include <cstdlib>
#include <iostream>
#include "fftw3.h"
#include "RingBuffer.h"

//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
//  Analysis Class (performs FFT using FFTW3)
//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
class Analysis{
public:
    enum class TRANSFORM{FFT, IFFT};
    enum class PARAMETER{REAL, IMAG, MAG, PHS, FRQ};
    enum class DATA{WAVEFORM, SPECTRUM};
    enum class WINDOW{HANN};
private:
    int samplingRate, windowSize, hopSize, paddedSize, numBins, numWrittenSinceFFT, appetite;
    bool padded;
    DATA state;
    WINDOW windowType;
    RingBuffer<float> * inputBuffer;
    RingBuffer<float> * outputBuffer;
    float * realBuffer;
    fftwf_complex * complexBuffer;
    fftwf_plan forwardPlan, backwardPlan;
    
    float * window;
    float * magnitudes;
    float * phases;
    float * frequencies;
public:
    
    Analysis();
    ~Analysis();
    
    //getters
    int getWindowSize() const{return windowSize;}
    int getNumBins() const{return numBins;}
    float get(const int index, const PARAMETER p) const;
    float getReal(const int index) const;
    float getImag(const int index) const;
    float getMag(const int index) const;
    float getPhs(const int index) const;
    float getFrq(const int index) const;
    float & getMagnitudes() const{return *magnitudes;}
    float & getPhases() const{return *phases;}
    float & getFrequencies() const{return *frequencies;}
    float & getMagnitudes() {return *magnitudes;}
    float & getPhases() {return *phases;}
    float & getFrequencies() {return *frequencies;}

    //setters
    void setWindow(const WINDOW w);
    void setComplex(const int index, const float realVal, const float imagVal = 0.0f);
    void set(const int index, const PARAMETER p, const float val = 0.0f);
    void setReal(const int index, const float val = 0.0f);
    void setImag(const int index, const float val = 0.0f);
    void setMag(const int index, const float val = 0.0f);
    void setPhs(const int index, const float val = 0.0f);
    
    //business & utility methods
    bool operator() (const float sample);//use this to write samples to the input buffer
    float operator() (void);//use this to read samples from the output buffer
    void transform(const TRANSFORM t);
    void updateSpectrum();
    void init(const WINDOW w = WINDOW::HANN, const int ws = 1024, const int sr = 44100, const bool p = true);
};

#endif  // ANALYSIS_H_INCLUDED
