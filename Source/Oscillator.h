/*
  ==============================================================================

    Oscillator.h
    Created: 8 Mar 2014 8:59:17pm
    Author:  Owen Campbell

  ==============================================================================
*/

#ifndef OSCILLATOR_H_INCLUDED
#define OSCILLATOR_H_INCLUDED

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
    int size;
    T * data;
    WAVEFORM type;
public:
    Wavetable(const WAVEFORM wf = WAVEFORM::SINE, const int s = 0){
        type = wf;
        size = s;
        data = new T[size];
        setWaveform(wf, true);
    }
    ~Wavetable(){
        delete[] data;
    }
    
    void setWaveform(const WAVEFORM wf, bool force){
        if(type != wf || force){
            type = wf;
            for(int i = 0; i < size; ++i){
                switch(type){
                    case WAVEFORM::SINE:
                        data[i] = sin(2.0 * M_PI * i / size);
                        break;
                    default:
                        data[i] = 0.0;
                        break;
                }
            }
        }
    }
};


template <class T>
class Oscillator {
private:
    int readPosA, readPosB;
    T samplingRate, amplitude, frequency, phase, targetAmplitude, targetFrequency, targetPhase, increment, fraction;
    Wavetable<T> * wavetable;
public:
    Oscillator(){
        wavetable = nullptr;
    };
    ~Oscillator(){
        wavetable = nullptr;
    };
    
    void init(Wavetable<T> &wt, const T sr = 44100, const T f = 1.0, const T p = 0.0){
        assert(p >= 0.0 && p <= 1.0);//p = [0.0, 1.0]
        assert(f > 0.0 && f <= sr / 2);//f = [0.0, Nyquist]
        
        wavetable = wt;
        setFrequency(f, sr);
        phase = p;
    }
    
    T next(){
        if(phase > 1.0){
            phase -= 1.0;
        }
        T location = phase * wavetable->size;
        readPosA = floor(location);
        if(readPosA == wavetable->size){
            readPosA = 0;
        }
        fraction = location - readPosA;
        readPosB = readPosA + 1;
        if(readPosB == wavetable->size){
            readPosB = 0;
        }
        T out = (1.0 - fraction) * wavetable->data[readPosA] +
        fraction * wavetable->data[readPosB];
        phase += increment;
        return out;
    }
    
    //getters
    const T getPhase(void) const{
        return phase;
    }
    const T getFrequency(void) const{
        return frequency;
    }
    
    //setters
    void setPhase(const T p){
        assert(p >= 0.0 && p <= 1.0);
        phase = p;
    }
    void setFrequency(const T f, const T sr = 44100){
        assert(f > 0.0 && f <= sr / 2);
        samplingRate = sr;
        frequency = f;
        increment = frequency * wavetable->size / samplingRate;
    }
    void setAmplitude(const T a){
        assert(a >= 0.0 && a <= 1.0);
        amplitude = a;
    }
    void setWavetable(Wavetable<T> &wt){
        wavetable = wt;
        setFrequency(frequency, samplingRate);
    }
};




#endif  // OSCILLATOR_H_INCLUDED
