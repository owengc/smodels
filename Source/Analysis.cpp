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
    window = new float[0];
    magnitudes = new float[0];
    phases = new float[0];
    frequencies = new float[0];
    
    //FFTW
    realBuffer = new float[0];
    complexBuffer = new fftwf_complex[0];
    forwardPlan = fftwf_plan_dft_r2c_1d(0, realBuffer, complexBuffer, FFTW_MEASURE);
    backwardPlan = fftwf_plan_dft_c2r_1d(0, complexBuffer, realBuffer, FFTW_MEASURE);
}
Analysis::~Analysis(){
    delete inputBuffer;
    delete outputBuffer;
    delete[] window;
    delete[] magnitudes;
    delete[] phases;
    delete[] frequencies;
    
    //FFTW
    fftwf_free(realBuffer);
    fftwf_free(complexBuffer);
    fftwf_destroy_plan(forwardPlan);
    fftwf_destroy_plan(backwardPlan);
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
        return complexBuffer[index][0];
    }
    catch(std::exception){
        std::cout << "Attempting to access out of range real number index." << std::endl;
        return 0.0f;
    }
}
float Analysis::getImag(const int index) const{
    try{
        return complexBuffer[index][1];
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
        complexBuffer[index][0] = realVal;
        complexBuffer[index][1] = imagVal;
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
        complexBuffer[index][0] = val;
    }
    catch(std::exception){
        std::cout << "Attempting to set out of range real number index." << std::endl;
    }
}
void Analysis::setImag(const int index, const float val){
    try{
        complexBuffer[index][1] = val;
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
        fftwf_execute(backwardPlan);//1 means inverse FFT
        state = DATA::WAVEFORM;
        for(int i = 0; i < windowSize; ++i){
            outputBuffer->write(realBuffer[i]);
        }
    }
    else{//FFT
        assert(state == DATA::WAVEFORM);
        if(appetite != hopSize){
            //after the first frame, we'll only need hopSize more samples to take another FFT
            appetite = hopSize; //putting this here so it won't have to check very often. might be a better way
        }
        //fill the real buffer with new input values
        for(int i = 0; i < windowSize; ++i){
            realBuffer[i] = inputBuffer->read() * window[i];//apply window function
        }
        fftwf_execute(forwardPlan);//0 means forward FFT
        state = DATA::SPECTRUM;
        updateSpectrum();//update mag, phs values in this frame for each bin
        numWrittenSinceFFT = 0;
    }
}


void Analysis::updateSpectrum(){
    assert(state == DATA::SPECTRUM);
    int i = 1; //ignoring dc & nyquist
    float real, imag, mag;
    for(; i < numBins; ++i){//before calculating magnitude, divide by windowSize and multiply by two
        real = complexBuffer[i][0] / (numBins - 1);
        imag = complexBuffer[i][1] / (numBins - 1);
        mag = sqrt(real * real + imag * imag);
        //if(mag > 0.1f)
        //{
        //    std::cout << "unscaled mag: " << mag << std::endl;
        //}
        //(float)i/numBins;//
        magnitudes[i] = mag;// / (numBins - 1); //using numBins because numBins == windowSize/2 + 1
        phases[i] = atan2f(imag, real) + M_PI;
    }
}

void Analysis::resize(const int wSize, const int sRate){
    state = DATA::WAVEFORM;
    sr = sRate;
    windowSize = wSize;
    hopSize = windowSize / 2;
    numBins = windowSize / 2 + 1;
    numWrittenSinceFFT = 0;
    appetite = windowSize;
    
    
    delete inputBuffer;
    inputBuffer = new RingBuffer<float>(windowSize);
    delete outputBuffer;
    outputBuffer = new RingBuffer<float>(windowSize);
    delete[] window;
    window = new float[windowSize]{1.0};
    delete[] magnitudes;
    magnitudes = new float[numBins]{0.0};
    delete[] phases;
    phases = new float[numBins]{0.0};
    delete[] frequencies;
    frequencies = new float[numBins];
    float scaleFactor = (float)sr / windowSize;
    for(int i = 0; i < numBins; ++i){
        frequencies[i] = i * scaleFactor;
    }
    setWindow(windowType);
    
    //FFTW
    fftwf_free(realBuffer);
    realBuffer = (float*) fftwf_malloc(sizeof(float) * windowSize);//new float[windowSize]{0.0};
    memset(realBuffer, 0, sizeof(float) * windowSize);
    fftwf_free(complexBuffer);
    complexBuffer = (fftwf_complex*) fftwf_alloc_complex(sizeof(fftwf_complex) * numBins);
    memset(complexBuffer, 0, sizeof(fftwf_complex) * numBins);
    fftwf_destroy_plan(forwardPlan);
    forwardPlan = fftwf_plan_dft_r2c_1d(windowSize, realBuffer, complexBuffer, FFTW_MEASURE);
    fftwf_destroy_plan(backwardPlan);
    backwardPlan = fftwf_plan_dft_c2r_1d(windowSize, complexBuffer, realBuffer, FFTW_MEASURE);
  /*
    fftw_complex *in, *out;
    4 fftw_plan my_plan;
    5 in = (fftw_complex*) fftw_malloc(sizeof(fftw_complex)*N);
    6 out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex)*N);
    7 my_plan = fftw_plan_dft_1d(N, in, out, FFTW_FORWARD, FFTW_ESTIMATE);
    8 fftw_execute(my_plan);
    10 fftw_destroy_plan(my_plan);
    11 fftw_free(in);
    12 fftw_free(out);
*/
}