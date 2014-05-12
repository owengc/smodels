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
    int readPosA, readPosB, interpDur, interpStep;
    T samplingRate, amplitude, frequency, phase, targetAmplitude, targetFrequency, currentAmplitude, currentFrequency, increment, sizeOverSr;
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
        //assert(sr / f <= wt->size && f <= sr / 2);//f = [0.0, Nyquist]
        
        samplingRate = sr;
        wavetable = wt;
        amplitude = targetAmplitude = a;
        frequency = targetFrequency = f;
        setFrequency(targetFrequency);
        phase = p;
        sizeOverSr = wavetable->size / samplingRate;
    }
    
    T next(){
        T out = 0.0, interpFactor = (T)interpStep / interpDur;
        T fraction;
        readPosA = (int)phase;
        readPosB = readPosA + 1;
        fraction = phase - readPosA;
        
        out = (1.0 - fraction) * wavetable->data[readPosA] +
              fraction * wavetable->data[readPosB];
        //std::cout << "Waveform value " <<  out << std::endl;
        //std::cout << "Oscillator output " <<  out * amplitude << std::endl;
        
        //interpolate frequency and amplitude:
        currentFrequency = interpFactor * frequency + (1.0 - interpFactor) * targetFrequency;
        currentAmplitude = interpFactor * amplitude + (1.0 - interpFactor) * targetAmplitude;
        increment = (currentFrequency * sizeOverSr);
        interpStep++;
        if(interpStep == interpDur){
            //std::cout << "wrapping interpolation" << std::endl;
            amplitude = targetAmplitude;
            frequency = targetFrequency;
        }
        
        //update phase, wrap
        phase += increment;
        while(phase >= wavetable->size){
            //std::cout << "Phase resetting (forward, before): " << phase << std::endl;
            phase -= wavetable->size;
            //std::cout << "Phase resetting (forward, after): " << phase << std::endl;
        }
        /*while(phase < 0.0){//only needed for FM, etc
            //std::cout << "Phase resetting (backward, before): " << phase << std::endl;
            phase += wavetable->size;
            //std::cout << "Phase resetting (backward, after): " << phase << std::endl;
        }*/
        //TODO: do linear interp between old values and new values, add new 'increment' vars to interp over hopsize samples
        //      also, look into time-frequency reassignment
        return out * currentAmplitude;
    }
    void update(const T a, const T f, const T p, const int i){
        interpDur = i;
        interpStep = 0;
        setAmplitude(a);
        setPhase(p);
        setFrequency(f);
    }

    void start(const T a, const T f, const T p){
        amplitude = currentAmplitude = targetAmplitude = a;
        phase = p;
        frequency = currentFrequency = targetFrequency = f;
    }
    void stop(){
        amplitude = currentAmplitude = targetAmplitude = 0;
    }
    
    //getters
    const T getAmplitude(void) const{
        return currentAmplitude;
    }
    const T getPhase(void) const{
        return phase;
    }
    const T getFrequency(void) const{
        return currentFrequency;
    }
    
    //setters
    void setPhase(const T p){
        assert(p >= 0.0 && p <= 1.0);
        phase = p * wavetable->size;
    }
    void setFrequency(const T f){
        //assert(samplingRate / f <= wavetable->size && f <= samplingRate / 2);
        targetFrequency = f;
    }
    void setAmplitude(const T a){
        assert(a >= 0.0 && a <= 1.0);
        targetAmplitude = a;
    }
    void setWavetable(Wavetable<T> &wt){
        wavetable = wt;
        setFrequency(targetFrequency);
    }
};




#endif  // OSCILLATOR_H_INCLUDED
