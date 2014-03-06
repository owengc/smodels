/*
  ==============================================================================

    Analysis.cpp
    Created: 1 Mar 2014 10:17:35pm
    Author:  Owen Campbell

  ==============================================================================
*/

#include "Analysis.h"
Analysis::Analysis(const WINDOW w){
    windowType = w;
    inputBuffer = new RingBuffer<float>(0);
    outputBuffer = new RingBuffer<float>(0);
    complexBuffer = new WDL_FFT_COMPLEX[0];
    window = new float[0];
    magnitudes = new float[0];
    phases = new float[0];
    frequencies = new float[0];
}
Analysis::~Analysis(){
    delete inputBuffer;
    delete outputBuffer;
    delete[] complexBuffer;
    delete[] window;
    delete[] magnitudes;
    delete[] phases;
    delete[] frequencies;
}

//getters
float Analysis::get(const int index, const PARAMETER p) const{
    switch(p){
        case PARAMETER::REAL:
            return getReal(index);
        case PARAMETER::IMAG:
            return getImag(index);
        case PARAMETER::MAG:
            return getMag(index);
        case PARAMETER::PHS:
            return getPhs(index);
        case PARAMETER::FRQ:
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
void Analysis::setWindow(const WINDOW w){//might support more options later
    for(int i = 0; i < windowSize; ++i){
        switch(w){
            case WINDOW::HANN:
                window[i] = 0.5 * (1 - cos(2 * M_PI * i / windowSize));
                break;
            default:
                std::cout << "invalid window type. defaulting to rectangle" << std::endl;
                window[i] = 1.0;
                break;
        }
    }
}

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
        case PARAMETER::REAL:
            return setReal(index, val);
        case PARAMETER::IMAG:
            return setImag(index, val);
        case PARAMETER::MAG:
            return setMag(index, val);
        case PARAMETER::PHS:
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
    inputBuffer->write(sample);//assuming normalized input
    numWrittenSinceFFT++;
    //return true once we've gotten enough new samples to take another FFT
    return (numWrittenSinceFFT == appetite)?true:false;
}

float Analysis::operator() (void){//use this to read samples from the output buffer
    return outputBuffer->read();
}

void Analysis::transform(const TRANSFORM t){
    if(t == TRANSFORM::IFFT){//IFFT
        assert(state == DATA::SPECTRUM);
        WDL_fft(complexBuffer, windowSize, (int)TRANSFORM::IFFT);//1 means inverse FFT
        state = DATA::WAVEFORM;
        for(int i = 0; i < windowSize; ++i){
            outputBuffer->write(complexBuffer[i].re);
        }
    }
    else{//FFT
        assert(state == DATA::WAVEFORM);
        if(appetite != hopSize){
            //after the first frame, we'll only need hopSize more samples to take another FFT
            appetite = hopSize; //putting this here so it won't have to check very often. might be a better way
        }
        //fill the complex buffer with new input values
        for(int i = 0; i < windowSize; ++i){
            setComplex(i, inputBuffer->read() * window[i]);//apply window function
        }
        WDL_fft(complexBuffer, windowSize, (int)TRANSFORM::FFT);//0 means forward FFT
        state = DATA::SPECTRUM;
        updateSpectrum();//update mag, phs values in this frame for each bin
        numWrittenSinceFFT = 0;
    }
}

void Analysis::updateSpectrum(){
    assert(state == DATA::SPECTRUM);
    int i = 0;
    float real, imag, mag;
    for(; i < numBins; ++i){//calc mag, divide by winSize and mult by two
        real = complexBuffer[i].re;
        imag = complexBuffer[i].im;
        mag = sqrt(real * real + imag * imag);// / (numBins - 1);
        if(mag > 1.0f){
            std::cout << "unscaled mag: " << mag << std::endl;
        }
        //(float)i/numBins;//
        magnitudes[i] = mag;// / (numBins - 1); //using numBins because numBins == windowSize/2 + 1
        phases[i] = atan2f(imag, real) + M_PI;
    }
}

void Analysis::resize(const int wSize, const int sRate){
    state = DATA::WAVEFORM;
    sr = sRate;
    windowSize = wSize;
    hopSize = wSize / 4;
    numBins = windowSize/2 + 1;
    numWrittenSinceFFT = 0;
    appetite = windowSize;
    
    
    delete inputBuffer;
    inputBuffer = new RingBuffer<float>(windowSize);
    delete outputBuffer;
    outputBuffer = new RingBuffer<float>(windowSize);
    
    delete[] complexBuffer;
    complexBuffer = new WDL_FFT_COMPLEX[windowSize];
    delete[] window;
    window = new float[windowSize]{1.0};
    delete[] magnitudes;
    magnitudes = new float[numBins]{0.5};
    delete[] phases;
    phases = new float[numBins]{0.0};
    delete[] frequencies;
    frequencies = new float[numBins];
    float scaleFactor = (float)sr / windowSize;
    for(int i = 0; i < numBins; ++i){
        frequencies[i] = i * scaleFactor;
    }
    setWindow(windowType);
}