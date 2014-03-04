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
#include "fft.h"


//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
//  Analysis Class (performs FFT using WDL wrapper for djbfft)
//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
enum TRANSFORM{
    FFT = 0,
    IFFT
};
enum PARAMETER{
    REAL = 0,
    IMAG,
    MAG,
    PHS,
    FRQ
};
enum MODE{
    WAVEFORM = 0,
    SPECTRUM
};
enum RANGE{
    NYQUIST = 0,
    ALL
};

class Analysis{
private:
    int sr, windowSize, hopSize, numBins, writePos, readPos, state;
    uint numFrames;
    WDL_FFT_COMPLEX * complexBuffer;
    float * ringBuffer;
    float * signalBuffer;
    float * magnitudes;
    float * phases;
    float * frequencies;
public:
    Analysis(){
        signalBuffer = new float[0];
        complexBuffer = new WDL_FFT_COMPLEX[0];
        magnitudes = new float[0];
        phases = new float[0];
        frequencies = new float[0];
    }
    ~Analysis(){
        delete[] signalBuffer;
        delete[] complexBuffer;
        delete[] magnitudes;
        delete[] phases;
        delete[] frequencies;
    }
    void resize(const int wSize = 2048, const int sRate = 44100){
        sr = sRate;
        writePos = 0;
        readPos = 0;
        windowSize = wSize;
        hopSize = wSize / 4;
        numBins = windowSize/2 + 1;
        delete[] complexBuffer;
        complexBuffer = new WDL_FFT_COMPLEX[windowSize];
        delete[] magnitudes;
        magnitudes = new float[numBins];
        std::fill(magnitudes, magnitudes + numBins, 0.0f);
        delete[] phases;
        phases = new float[numBins];
        std::fill(phases, phases + numBins, 0.0f);
        delete[] frequencies;
        frequencies = new float[numBins];
        float scaleFactor = (float)sr / windowSize;
        for(int i = 0; i < numBins; ++i){
            frequencies[i] = i * scaleFactor;
        }
    }
    void init(){
        WDL_fft_init();
    }
    
    //getters
    int getWindowSize() const{
        return windowSize;
    }
    int getNumBins() const{
        return numBins;
    }
    int getState() const{
        return state;
    }
    float get(const int index, const PARAMETER p) const;
    float getReal(const int index) const;
    float getImag(const int index) const;
    float getMag(const int index) const;
    float getPhs(const int index) const;
    float getFrq(const int index) const;

    //setters
    void setComplex(const int index, const float realVal, const float imagVal = 0.0f){
        try{
            complexBuffer[index].re = realVal;
            complexBuffer[index].im = imagVal;
        }
        catch(std::exception){
            std::cout << "Attempting to set out of range complex number index." << std::endl;
        }
    }
    void set(const int index, const PARAMETER p, const float val = 0.0f);
    void setReal(const int index, const float val = 0.0f);
    void setImag(const int index, const float val = 0.0f);
    void setMag(const int index, const float val = 0.0f);
    void setPhs(const int index, const float val = 0.0f);
    
    //business methods
    bool operator() (const float sample){//use this to write real-valued samples to the analysis buffer
        assert(state == WAVEFORM);
        setComplex(writePos++, sample / MAXFLOAT);//normalize input
        if(writePos == windowSize){
            writePos = 0;
            return true;
        }
        else{
            return false;
        }
    }
    float operator() (void){//use this to read real-valued samples from the analysis buffer
        assert(state == WAVEFORM);
        if(readPos == windowSize - 1){
            readPos = 0;
        }
        return complexBuffer[readPos++].re;
    }
    void transform(const TRANSFORM t){
        if(t == IFFT){//IFFT
            assert(state == SPECTRUM);
            WDL_fft(complexBuffer, windowSize, IFFT);//1 means inverse FFT
            state = WAVEFORM;
        }
        else{//FFT
            assert(state == WAVEFORM);
            WDL_fft(complexBuffer, windowSize, FFT);
            state = SPECTRUM;
            //updateSpectrum(numFrames++);
            
        }
    }
    /*void updateSpectrum(){
        assert(state == SPECTRUM);
        for(int i = 0; i < numBins; i++){//calc mag, divide by winSize and mult by two
            
        }
    }*/
    
    /*
    
    void exportComplex(WDL_FFT_COMPLEX * outputBuffer, const int range){
        //range: NYQUIST = Only export bins up to and including Nyquist
        //      ALL     = Export all bins, even negative frequencies
        int numValues = (range == NYQUIST)?numBins:windowSize + 1;
        std::copy_n(complexBuffer, numValues, outputBuffer);
    }
    void exportReal(float * outputBuffer, const int range){
        //range: NYQUIST = Only export bins up to and including Nyquist
        //      ALL     = Export all bins, even negative frequencies
        int numValues = (range == NYQUIST)?numBins:windowSize + 1;
        for(int i = 0; i < numValues; ++i){
            outputBuffer[i] = complexBuffer[i].re;
        }
    }
    void exportImag(float * outputBuffer, const int range){
        //range: NYQUIST = Only export bins up to and including Nyquist
        //      ALL     = Export all bins, even negative frequencies
        int numValues = (range == NYQUIST)?numBins:windowSize + 1;
        for(int i = 0; i < numValues; ++i){
            outputBuffer[i] = complexBuffer[i].im;
        }
    }*/
};

#endif  // ANALYSIS_H_INCLUDED
