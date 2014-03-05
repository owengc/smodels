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
    int size, readPos, writePos, numWrittenSinceRead;
    T * data;
public:
    RingBuffer(const int s = 0);
    ~RingBuffer();
    T read() const;
    void write(const T x);
    int getNumWrittenSinceRead() const;
};



#endif  // RINGBUFFER_H_INCLUDED
