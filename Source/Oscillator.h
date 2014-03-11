/*
  ==============================================================================

    Oscillator.h
    Created: 8 Mar 2014 8:59:17pm
    Author:  Owen Campbell

  ==============================================================================
*/

#ifndef OSCILLATOR_H_INCLUDED
#define OSCILLATOR_H_INCLUDED

#include <assert>
#include <cmath>

template <class T>
class Oscillator {
    
enum class WAVEFORM{SINE}
private:
    int size, samplingRate, readPosA, readPosB;
    T frequency, phase, increment, fraction;
    WAVEFORM type;
    T * data;
public:
    Oscillator(const int s = 0, const int sr = 44100){
        size = s;
        samplingRate = sr;
        data = new T[size];
        readPosA = 0;
        readPosB = 1;
    }
    ~Oscillator(){
        delete[] data;
    }
    void init(const WAVEFORM w, const T f, const T p){//p = [0.0, 1.0]
        assert(p >= 0.0 && p <= 1.0);
        frequency = f;
        phase = p;
        increment = frequency * size / samplingRate;
        readPosA = floor(phase * size);
        fraction = phase * size - readPosA;
        readPosB = readPosA + 1;
        for(int i = 0; i < size; ++i){
            switch(w){
                case SINE:
                    data[i] = sin(2.0 * M_PI * i / size);
                    break;
                default:
                    data[i] = 0.0;
                    break;
            }
        }
    }
    T next(){
        if(phase > 1.0){
            phase -= 1.0;
        }
        T location = phase * size;
        readPosA = floor(location);
        if(readPosA == size){
            readPosA = 0;
        }
        fraction = location - readPosA;
        readPosB = readPosA + 1;
        if(readPosB == size){
            readPosB = 0;
        }
        T out = (1.0 - fraction) * data[readPosA] +
        fraction * data[readPosB];
        phase += increment;
        return out;
    }
    void setPhase(const T p){
        assert(p >= 0.0 && p <= 1.0);
        phase = p;
    }
    void setFrequency(const T f){
        assert(f >= 0 && f <= samplingRate / 2);
        frequency = f;
        increment = frequency * size / samplingRate;
    }
};




#endif  // OSCILLATOR_H_INCLUDED
