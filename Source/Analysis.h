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
    enum class PARAMETER{REAL, IMAG, AMP, MAG, PHS, FRQ, RMS};
    enum class WINDOW{HANN, GAUSSIAN};
private:
    int samplingRate, windowSize, hopSize, hopFactor, paddedSize, numBins, numWrittenSinceFFT, appetite;
    float rms, normFactor, denormFactor, samplingRateOverSize;
    bool padded;
    WINDOW windowType;
    RingBuffer<float> * inputBuffer;
    RingBuffer<float> * outputBuffer;
    float * realBuffer;
    fftwf_complex * complexBuffer;
    fftwf_plan forwardPlan, backwardPlan;
    
    float * window;
	float * amplitudes;
    float * magnitudes;
    float * phases;
    float * frequencies;
public:
    Analysis(const WINDOW w = WINDOW::HANN, const int ws = 1024, const int hf = 4, const int sr = 44100, const bool p = true);
    ~Analysis();
    
    //getters
    int getWindowSize() const{return windowSize;}
    int getNumBins() const{return numBins;}
    int getAppetite() const{return appetite;}
	float getRMS() const{return rms;}
    float getNormFactor() const{return normFactor;}
	float getDenormFactor() const{return denormFactor;}
	float getSamplingRateOverSize() const{return samplingRateOverSize;}
    float & getAmplitudes() const{return *amplitudes;}
    float & getMagnitudes() const{return *magnitudes;}
    float & getPhases() const{return *phases;}
    float & getFrequencies() const{return *frequencies;}
	float & getAmplitudes() {return *amplitudes;}
    float & getMagnitudes() {return *magnitudes;}
    float & getPhases() {return *phases;}
    float & getFrequencies() {return *frequencies;}

    //setters
    void setWindow(const WINDOW w);
    
    //business & utility methods
    bool operator() (const float sample);//use this to write samples to the input buffer
    float operator() (void);//use this to read samples from the output buffer
    void transform(const TRANSFORM t);
    void updateSpectrum();
    void init();
};

#endif  // ANALYSIS_H_INCLUDED
