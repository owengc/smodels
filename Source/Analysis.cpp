/*
  ==============================================================================

    Analysis.cpp
    Created: 1 Mar 2014 10:17:35pm
    Author:  Owen Campbell

  ==============================================================================
*/
#include "Analysis.h"
#define CRUMB 0.0000001
Analysis::Analysis(const WINDOW w, const int ws, const int hf, const int sr, const bool p){
    windowType = w;
    padded = p;
    samplingRate = sr;
    windowSize = ws;
    hopFactor = hf;
    hopSize = windowSize / hopFactor;//TODO: test if shrinking hop size will improve sound quality
    paddedSize = (padded)?windowSize * 2:windowSize;//zero padding
    numBins = paddedSize / 2 + 1;
    numWrittenSinceFFT = 0;
    appetite = windowSize;
    
    inputBuffer = new RingBuffer<float>(windowSize);
    outputBuffer = new RingBuffer<float>(windowSize);
    window = new float[windowSize]{0.0};
	amplitudes = new float[numBins]{0.0};
    magnitudes = new float[numBins]{0.0};
    phases = new float[numBins]{0.0};
    frequencies = new float[numBins];
    samplingRateOverSize = (float)samplingRate / paddedSize;
    for(int i = 0; i < numBins; ++i){
        frequencies[i] = i * samplingRateOverSize;
        //std::cout << "Bin " << i << " frq: " << frequencies[i] << std::endl;
    }
    setWindow(windowType);
    
    //FFTW
    realBuffer = (float*) fftwf_malloc(sizeof(float) * paddedSize);
    memset(realBuffer, 0, sizeof(float) * paddedSize);
    complexBuffer = (fftwf_complex*) fftwf_alloc_complex(sizeof(fftwf_complex) * numBins);
    memset(complexBuffer, 0, sizeof(fftwf_complex) * numBins);
    forwardPlan = fftwf_plan_dft_r2c_1d(paddedSize, realBuffer, complexBuffer, FFTW_MEASURE);
    backwardPlan = fftwf_plan_dft_c2r_1d(paddedSize, complexBuffer, realBuffer, FFTW_MEASURE);
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
	float sigma, denom;
    for(int i = 0; i < windowSize; ++i){
        switch(w){
            case WINDOW::HANN:
                window[i] = 0.5 * (1 - cos(2 * M_PI * i / float(windowSize)));
                break;
			case WINDOW::GAUSSIAN:
				sigma = windowSize / 8;
				denom = 1.0 / (2 * sigma * sigma);
				window[i] = expf(-((windowSize - i) * (windowSize - i)) * denom);
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
        fftwf_execute(backwardPlan);//1 means inverse FFT
        for(int i = 0; i < windowSize; ++i){
            outputBuffer->write(realBuffer[i]);
        }
    }
    else{//FFT
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
        numWrittenSinceFFT = 0;
        updateSpectrum();//update mag, phs values in this frame for each bin
    }
}


void Analysis::updateSpectrum(){
    int i = 1; //ignoring dc & nyquist
    float real, imag, amp, maxAmp = -MAXFLOAT, scaleFactor = 1.0 / (numBins - 1);
    for(; i < numBins - 1; ++i){//before calculating magnitude, divide by windowSize and multiply by two
        real = complexBuffer[i][0];
        imag = complexBuffer[i][1];
		amp = 2.0 * sqrt(real * real + imag * imag) * scaleFactor;
        if(amp > maxAmp){
            maxAmp = amp;
			//std::cout << "max amp: " << maxMag << std::endl;
        }
		//std::cout << "amp: " << amp << std::endl;
		amplitudes[i] = amp;
        phases[i] = (atan2f(imag, real) + M_PI) / (2.0 * M_PI);
    }
	//normalize magnitudes and convert to dB
	normFactor = 1.0 / maxAmp;
	denormFactor = maxAmp;
	for(i = 1; i < numBins - 1; ++i){
		magnitudes[i] = 20.0 * log10f(amplitudes[i] /* normFactor*/ + CRUMB);
		//std::cout << "mag: " << magnitudes[i] << std::endl;
	}
}

void Analysis::init(){
    memset(realBuffer, 0, sizeof(float) * paddedSize);
    memset(complexBuffer, 0, sizeof(float) * numBins);
    numWrittenSinceFFT = 0;
}