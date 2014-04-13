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
        memset(realBuffer, 0, sizeof(float) * paddedSize);
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
    maxMag = 0;
    for(; i < numBins; ++i){//before calculating magnitude, divide by windowSize and multiply by two
        real = complexBuffer[i][0] / (numBins - 1);
        imag = complexBuffer[i][1] / (numBins - 1);
        //mag = 20.0 * log10f(sqrt(real * real + imag * imag));
        mag = sqrt(real * real + imag * imag);
        magnitudes[i] = mag;
        if(mag > maxMag){
            maxMag = mag;
        }
        phases[i] = (atan2f(imag, real) + M_PI) / (2.0 * M_PI);
    }
}

void Analysis::init(const WINDOW w, const int ws, const int sr, const bool p){
    state = DATA::WAVEFORM;
    windowType = w;
    padded = p;
    samplingRate = sr;
    windowSize = ws;
    hopSize = windowSize / 2;
    paddedSize = (padded)?windowSize * 2:windowSize;//zero padding
    numBins = paddedSize / 2 + 1;
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
    float scaleFactor = (float)samplingRate / paddedSize;
    for(int i = 0; i < numBins; ++i){
        frequencies[i] = i * scaleFactor;
        //std::cout << "Bin " << i << " frq: " << frequencies[i] << std::endl;
    }
    setWindow(windowType);
    
    //FFTW
    fftwf_free(realBuffer);
    realBuffer = (float*) fftwf_malloc(sizeof(float) * paddedSize);
    memset(realBuffer, 0, sizeof(float) * paddedSize);
    fftwf_free(complexBuffer);
    complexBuffer = (fftwf_complex*) fftwf_alloc_complex(sizeof(fftwf_complex) * numBins);
    memset(complexBuffer, 0, sizeof(fftwf_complex) * numBins);
    fftwf_destroy_plan(forwardPlan);
    forwardPlan = fftwf_plan_dft_r2c_1d(paddedSize, realBuffer, complexBuffer, FFTW_MEASURE);
    fftwf_destroy_plan(backwardPlan);
    backwardPlan = fftwf_plan_dft_c2r_1d(paddedSize, complexBuffer, realBuffer, FFTW_MEASURE);
}