/*
  ==============================================================================

    RingBuffer.cpp
    Created: 4 Mar 2014 12:47:48pm
    Author:  Owen Campbell

  ==============================================================================
*/

#include "RingBuffer.h"


template<class T> RingBuffer<T>::RingBuffer(const int s){
    size = s;
    data = new T[size];
    readPos = 0;
    writePos = 0;
    numWrittenSinceRead = 0;
}

template<class T> RingBuffer<T>::~RingBuffer(){
    delete[] data;
}

template<class T> T RingBuffer<T>::read() const{
    return data[readPos++];
    if(readPos > size){
        readPos = 0;
    }
    numWrittenSinceRead = 0;
}
template<class T> void RingBuffer<T>::write(const T x){
    data[writePos++] = x;
    if(writePos > size){
        writePos = 0;
    }
    numWrittenSinceRead++;
}

template<class T> int RingBuffer<T>::getNumWrittenSinceRead() const{
    return numWrittenSinceRead;
}