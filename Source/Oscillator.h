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
    int size, harmonics;
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
            int i = 0;
            for(; i < size; ++i){
                data[i] = 0;//resetting table
            }
            switch(type){
                case WAVEFORM::SINE:
                    generator = &Wavetable::generateSine;
                    break;
                default:
                    break;
            }
            for(i = 0; i < size; ++i){
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
    int readPosA, readPosB;
    T samplingRate, amplitude, frequency, phase, targetAmplitude, targetFrequency, targetPhase, increment, sizeOverSr;
    Wavetable<T> * wavetable;
public:
    Oscillator(){
        wavetable = nullptr;
    };
    ~Oscillator(){
        wavetable = nullptr;
    };
    
    void init(Wavetable<T> * wt, const T sr = 44100, const T a = 0.1, const T f = 440.0, const T p = 0.0){
        assert(p >= 0.0 && p <= 1.0);//p = [0.0, 1.0]
        assert(sr / f <= wt->size && f <= sr / 2);//f = [0.0, Nyquist]
        
        samplingRate = sr;
        wavetable = wt;
        amplitude = a;
        increment = 0;//this serves as a flag
        setFrequency(f);
        phase = p;
        sizeOverSr = wavetable->size / samplingRate;
    }
    
    T next(){
        T out = 0.0;
        double fraction;
        readPosA = (int)phase;
        readPosB = readPosA + 1;
        fraction = phase - readPosA;

        out = (1.0 - fraction) * wavetable->data[readPosA] +
              fraction * wavetable->data[readPosB];
        //std::cout << "Waveform value " <<  out << std::endl;
        //std::cout << "Oscillator output " <<  out * amplitude << std::endl;
        
        //update phase, wrap
        phase += increment;
        while(phase >= wavetable->size){
            //std::cout << "Phase resetting (forward, before): " << phase << std::endl;
            phase -= wavetable->size;
            //std::cout << "Phase resetting (forward, after): " << phase << std::endl;
        }
        while(phase < 0.0){
            //std::cout << "Phase resetting (backward, before): " << phase << std::endl;
            phase += wavetable->size;
            //std::cout << "Phase resetting (backward, after): " << phase << std::endl;
        }
        return out * amplitude;
    }
    void update(const T a, const T f, const T p){
        setAmplitude(a);
        setPhase(p);
        setFrequency(f);
    }

    void start(const T a, const T f, const T p){
        amplitude = a;
        phase = p;
        increment = 0;//this serves as a flag
        setFrequency(f);
    }
    void stop(){
        amplitude = 0;
    }
    
    //getters
    const T getAmplitude(void) const{
        return amplitude;
    }
    const T getPhase(void) const{
        return phase;
    }
    const T getFrequency(void) const{
        return frequency;
    }
    
    //setters
    void setPhase(const T p){
        assert(p >= 0.0 && p <= 1.0);
        phase = (phase * 0.8) + (p * wavetable->size * 0.2);
    }
    void setFrequency(const T f){
        //assert(samplingRate / f <= wavetable->size && f <= samplingRate / 2);
        frequency = f;
        if(increment == 0){//do not interpolate at initialization
            increment = (frequency * sizeOverSr);
        }
        else{//interpolate between old value and new value
            increment = (increment * 0.7) + ((frequency * sizeOverSr) * 0.3);
        }
    }
    void setAmplitude(const T a){
        assert(a >= 0.0 && a <= 1.0);
        amplitude = (amplitude * 0.6) + (a * 0.4);
    }
    void setWavetable(Wavetable<T> &wt){
        wavetable = wt;
        increment = 0;
        setFrequency(frequency);
    }
};




#endif  // OSCILLATOR_H_INCLUDED
