/*
  ==============================================================================

    Noise.h
    Created: 16 Mar 2014 12:17:56pm
    Author:  Owen Campbell

  ==============================================================================
*/

#ifndef NOISE_H_INCLUDED
#define NOISE_H_INCLUDED

#include "RingBuffer.h"
#include <ctime>
#include <random>//C++11
#include <cassert>

//sample-and-hold 6 octave pink noise generator based on algorithm described here: http://www.firstpr.com.au/dsp/pink-noise/
//<random> usage adapted from example here: http://choorucode.com/2010/11/24/c-random-number-generation-using-random/
template <class T>
class PinkNoise{
private:
    typedef std::minstd_rand                               Engine;
    typedef std::uniform_real_distribution<T>        Distribution;//might not work for ints?
    typedef std::default_random_engine                  Generator;
    const int octaves;
    int * counters, * timers;
    Generator * generator;
    T * values;
public:
    PinkNoise(){
        values = nullptr;
        counters = nullptr;
        timers = nullptr;
        generator = nullptr;
    }
    ~PinkNoise(){
        delete[] values;
        delete[] counters;
        delete[] timers;
        delete generator;
    }
    T getRandom(){
        return (T)(*generator)();
    }
    void init(const T min, const T max){
        generator = new Generator(Engine(time(0)), Distribution(min, max));
        octaves = 6;//0th octave is full-on white noise
        values = new T[octaves];
        counters = new int[octaves];
        timers = new int[octaves];
        for(int i = 0; i < octaves; ++i){
            values[i] = getRandom();//initialize all octaves with random values
            timers[i] = pow(2, i);//establish sampling delays
            counters[i] = (i<=1)?0:timers[i-1] + 1;//stagger initializations so exactly 2 numbers are generated at each step
        }
    }
    T next(){
        T out = 0;
        int numUpdates = 0;
        for(int i = 0; i < octaves; ++i){
            out += values[i] / octaves;
            counters[i]++;
            if(counters[i] == timers[i]){
                counters[i] = 0;
                values[i] = getRandom();
                numUpdates++;
            }
        }
        assert(numUpdates == 2);//for testing
        return out;
    }
};



#endif  // NOISE_H_INCLUDED
