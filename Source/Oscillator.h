 /*
  ==============================================================================

    Oscillator.h
    Created: 8 Mar 2014 8:59:17pm
    Author:  Owen Campbell

  ==============================================================================
*/

#ifndef OSCILLATOR_H_INCLUDED
#define OSCILLATOR_H_INCLUDED

#ifndef M_PI
#define M_PI (3.1415926535897932)
#endif
#define TWOPI (2.0 * M_PI)

/* max tablelength = 4294967296 = 2^^32*/
#define MAXTABLESIZE 0x10000000
#define PHASEMASK 0x0FFFFFFF
//00010000000000000000000000000000
//00001111111111111111111111111111
#include <cassert>
#include <cmath>

template <class T>
class Oscillator;

template <class T>
class Wavetable {
friend class Oscillator<T>;
public:
    enum class WAVEFORM{SINE};
private:
    uint32_t size, harmonics;
    T * data, step;
    WAVEFORM type;
    T (Wavetable::*generator)(const int index) = nullptr;
public:
    Wavetable(const WAVEFORM wf = WAVEFORM::SINE, const int s = 2048, const int h = 1){
        assert(s > 0 && h >= 1 && h <= s / 2);
        size = s;
        step = TWOPI / size;
        harmonics = h;
        data = new T[size + 1];//padding with 'guard point' for phase wrapping
        setWaveform(wf, true);
    }
    ~Wavetable(){
        delete[] data;
    }
    
    void setWaveform(const WAVEFORM wf, bool force){
        if(type != wf || force){
            type = wf;
            memset(data, 0, sizeof(T));
            switch(type){
                case WAVEFORM::SINE:
                    generator = &Wavetable::generateSine;
                    break;
                default:
                    break;
            }
            for(int i = 0; i < size; ++i){
                data[i] = (this->*generator)(i);
                //std::cout << "writing to wavetable: " << data[i] << std::endl;
            }
            data[size] = data[0];//set 'guard point' to be same as first value
        }
    }
    
    T generateSine(const int index){
        return sin(index * step);
    }
};


template <class T>
class Oscillator {
private:
    uint32_t phase, wavetableSize, hiMask, loMask, loBits, loPhase, increment, readPos, interpDur, interpPhase, interpInc;
    T samplingRate, amplitude, frequency, targetAmplitude, targetFrequency, currentAmplitude, currentFrequency, loDivide, interpDivide, samplingInterval;
    Wavetable<T> * wavetable;
public:
    Oscillator(){
        wavetable = nullptr;
    };
    ~Oscillator(){
        wavetable = nullptr;
    };
    
    void init(Wavetable<T> * wt, const T sr = 44100){
        uint32_t sizeTest, loMod;
        samplingRate = sr;
        wavetable = wt;
        wavetableSize = wavetable->size;
        for(sizeTest = wavetableSize, loBits = 0; (sizeTest & MAXTABLESIZE) == 0; loBits++, sizeTest <<= 1){
            ;
        }
        assert(sizeTest == MAXTABLESIZE);//makes sure wavetable size is a power of two
        hiMask = wavetableSize - 1;
        loMod = MAXTABLESIZE / wavetableSize;
        loMask = loMod - 1;
        loDivide = 1.0 / (T)loMod;
        samplingInterval = MAXTABLESIZE / samplingRate;
        interpDivide = 1.0 / (T)MAXTABLESIZE;
    }
    
    T next(){
        T out = 0.0, interpFraction = interpPhase * interpDivide, oneMinusInterpFraction = 1.0 - interpFraction, fraction = (phase & loMask) * loDivide;
        readPos = phase >> loBits;
	
        out = (1.0 - fraction) * wavetable->data[readPos] +
              fraction * wavetable->data[readPos + 1];
        
        //interpolate frequency and amplitude:
		currentAmplitude = interpFraction * amplitude + oneMinusInterpFraction * targetAmplitude;
		currentFrequency = interpFraction * frequency + oneMinusInterpFraction * targetFrequency;
		increment = (uint32_t)(currentFrequency * samplingInterval);
		
        interpPhase += interpInc;
		interpPhase &= PHASEMASK;
        //update phase, wrap
        phase += increment;
        phase &= PHASEMASK;
        //TODO: look into time-frequency reassignment
        return out * currentAmplitude;
    }
    void update(const T a, const T f, const T p, const int i){
        interpDur = i;
        interpPhase = 0;
		interpInc = MAXTABLESIZE / interpDur;
        setAmplitude(a);
        setFrequency(f);
    }

    void start(const T a, const T f, const T p){
		phase = 0;
        amplitude = currentAmplitude = targetAmplitude = a;
        frequency = currentFrequency = targetFrequency = f;
		increment = (uint32_t)(currentFrequency * samplingInterval);
    }
    void stop(){
        amplitude = currentAmplitude = targetAmplitude = 0;
    }
    
    //getters
    const T getAmplitude(void) const{
        return currentAmplitude;
    }
    const T getFrequency(void) const{
        return currentFrequency;
    }
    
    //setters
    void setFrequency(const T f){
        //assert(samplingRate / f <= wavetable->size && f <= samplingRate / 2);
		frequency = currentFrequency;
        targetFrequency = f;
    }
    void setAmplitude(const T a){
        assert(a >= 0.0 && a <= 1.0);
		amplitude = currentAmplitude;
        targetAmplitude = a;
    }
    void setWavetable(Wavetable<T> &wt){
        wavetable = wt;
        setFrequency(targetFrequency);
    }
};




#endif  // OSCILLATOR_H_INCLUDED
