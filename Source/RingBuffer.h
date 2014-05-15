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
    uint32_t size, mask, readPos, writePos;
    T * data;
public:
    RingBuffer(const int s = 0){
        size = s; //size must be power of two!
		mask = size - 1;
		//example: size = 1024
		//size: 00000000000000000000010000000000
		//mask: 00000000000000000000001111111111
        data = new T[size];
        readPos = 0;
        writePos = 0;
    }
    ~RingBuffer(){
        delete[] data;
    }
    T read(){
		readPos &= mask;
        return data[readPos++];
    }
    void write(const T x){
		writePos &= mask;
        data[writePos++] = x;
    }
};



#endif  // RINGBUFFER_H_INCLUDED
