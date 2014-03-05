/*
  ==============================================================================

    Analysis.cpp
    Created: 1 Mar 2014 10:17:35pm
    Author:  Owen Campbell

  ==============================================================================
*/

#include "Analysis.h"
Analysis::Analysis(){
    inputBuffer = new RingBuffer<float>(0);
    outputBuffer = new RingBuffer<float>(0);
    complexBuffer = new WDL_FFT_COMPLEX[0];
    magnitudes = new float[0];
    phases = new float[0];
    frequencies = new float[0];
}
Analysis::~Analysis(){
    delete inputBuffer;
    delete outputBuffer;
    delete[] complexBuffer;
    delete[] magnitudes;
    delete[] phases;
    delete[] frequencies;
}

//getters
float Analysis::get(const int index, const PARAMETER p) const{
    switch(p){
        case REAL:
            return getReal(index);
        case IMAG:
            return getImag(index);
        case MAG:
            return getMag(index);
        case PHS:
            return getPhs(index);
        case FRQ:
            return getFrq(index);
        default:
            std::cout << "Analysis.get called with invalid parameter" << std::endl;
            return 0.0f;
    }
}
float Analysis::getReal(const int index) const{
    try{
        return complexBuffer[index].re;
    }
    catch(std::exception){
        std::cout << "Attempting to access out of range real number index." << std::endl;
        return 0.0f;
    }
}
float Analysis::getImag(const int index) const{
    try{
        return complexBuffer[index].im;
    }
    catch(std::exception){
        std::cout << "Attempting to access out of range imaginary number index." << std::endl;
        return 0.0f;
    }
}
float Analysis::getMag(const int index) const{
    try{
        return magnitudes[index];
    }
    catch(std::exception){
        std::cout << "Attempting to access out of range magnitude index." << std::endl;
        return 0.0f;
    }
}
float Analysis::getPhs(const int index) const{
    try{
        return phases[index];
    }
    catch(std::exception){
        std::cout << "Attempting to access out of range phs index." << std::endl;
        return 0.0f;
    }
}
float Analysis::getFrq(const int index) const{
    try{
        return frequencies[index];
    }
    catch(std::exception){
        std::cout << "Attempting to access out of range frequency index." << std::endl;
        return 0.0f;
    }
}

//setters
void Analysis::setComplex(const int index, const float realVal, const float imagVal){
    try{
        complexBuffer[index].re = realVal;
        complexBuffer[index].im = imagVal;
    }
    catch(std::exception){
        std::cout << "Attempting to set out of range complex number index." << std::endl;
    }
}
void Analysis::set(const int index, const PARAMETER p, const float val){
    switch(p){
        case REAL:
            return setReal(index, val);
        case IMAG:
            return setImag(index, val);
        case MAG:
            return setMag(index, val);
        case PHS:
            return setPhs(index, val);
        default:
            std::cout << "Analysis.set called with invalid parameter" << std::endl;
    }
}
void Analysis::setReal(const int index, const float val){
    try{
        complexBuffer[index].re = val;
    }
    catch(std::exception){
        std::cout << "Attempting to set out of range real number index." << std::endl;
    }
}
void Analysis::setImag(const int index, const float val){
    try{
        complexBuffer[index].im = val;
    }
    catch(std::exception){
        std::cout << "Attempting to set out of range imaginary number index." << std::endl;
    }
}
void Analysis::setMag(const int index, const float val){
    try{
        magnitudes[index] = val;
    }
    catch(std::exception){
        std::cout << "Attempting to set out of range magnitude index." << std::endl;
    }
}
void Analysis::setPhs(const int index, const float val){
    try{
        magnitudes[index] = val;
    }
    catch(std::exception){
        std::cout << "Attempting to set out of range magnitude index." << std::endl;
    }
}

//business methods
bool Analysis::operator() (const float sample){//use this to write samples to the input buffer
    assert(state == WAVEFORM);
    inputBuffer->write(sample / MAXFLOAT);//normalize input
    //return true once we've gotten enough new samples to take another FFT
    return (inputBuffer->getNumWrittenSinceRead() == appetite)?true:false;
}

float Analysis::operator() (void){//use this to read samples from the output buffer
    assert(state == WAVEFORM);
    return outputBuffer->read();
}

void Analysis::transform(const TRANSFORM t){
    if(t == IFFT){//IFFT
        assert(state == SPECTRUM);
        WDL_fft(complexBuffer, windowSize, IFFT);//1 means inverse FFT
        state = WAVEFORM;
        for(int i = 0; i < windowSize; ++i){
            outputBuffer->write(complexBuffer[i].re);
        }
    }
    else{//FFT
        assert(state == WAVEFORM);
        if(appetite != hopSize){
            //after the first frame, we'll only need hopSize more samples to take another FFT
            appetite = hopSize; //putting this here so it won't have to run as often.
        }
        //fill the complex buffer with new input values
        for(int i = 0; i < windowSize; ++i){
            setComplex(i, inputBuffer->read());
        }
        WDL_fft(complexBuffer, windowSize, FFT);//0 means forward FFT
        state = SPECTRUM;
        updateSpectrum();//update mag, phs values in this frame for each bin
    }
}

void Analysis::updateSpectrum(){
    assert(state == SPECTRUM);
    int i, real, imag;
    for(i = 0; i < numBins; ++i){//calc mag, divide by winSize and mult by two
        real = complexBuffer[i].re;
        imag = complexBuffer[i].im;
        magnitudes[i] = sqrt(real * real + imag * imag) / numBins; //using numBins because numBins == windowSize / 2
        phases[i] = atan2f(imag, real) + M_PI;
    }
}

void Analysis::resize(const int wSize, const int sRate){
    sr = sRate;
    windowSize = wSize;
    hopSize = wSize / 4;
    numBins = windowSize/2 + 1;
    appetite = windowSize;
    
    delete inputBuffer;
    inputBuffer = new RingBuffer<float>(windowSize);
    delete outputBuffer;
    outputBuffer = new RingBuffer<float>(windowSize);
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