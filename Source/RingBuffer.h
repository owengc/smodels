/*
  ==============================================================================

    RingBuffer.h
    Created: 4 Mar 2014 12:47:48pm
    Author:  Owen Campbell

  ==============================================================================
*/

#ifndef RINGBUFFER_H_INCLUDED
#define RINGBUFFER_H_INCLUDED

template <class T>
class RingBuffer {
private:
    int size, readPos, writePos;
    T * data;
public:
    RingBuffer(const int s = 0){
        size = s;
        data = new T[size];
        readPos = 0;
        writePos = 0;
    }
    ~RingBuffer(){
        delete[] data;
    }
    T read(){
        if(readPos == size){
            readPos = 0;
        }
        return data[readPos++];
    }
    void write(const T x){
        if(writePos == size){
            writePos = 0;
        }
        data[writePos++] = x;
    }
};



#endif  // RINGBUFFER_H_INCLUDED
