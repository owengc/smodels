/*
  ==============================================================================

    Track.h
    Created: 15 Mar 2014 5:34:50pm
    Author:  Owen Campbell

  ==============================================================================
*/

#ifndef TRACK_H_INCLUDED
#define TRACK_H_INCLUDED

class SinusoidalModel;

class Track{
friend class SinusoidalModel;
public:
    enum class STATUS{ALIVE, DEAD, LIMBO};
private:
    float amp, frq, phs, lastAmp, lastFrq, lastPhs;
    STATUS status;
    int limboFrames;
    SinusoidalModel * model;
public:
    Track(){
        status = STATUS::DEAD;
        model = nullptr;
    };
    ~Track(){
        model = nullptr;
    };
    
    void init(SinusoidalModel * m, const float a, const float f, const float p);//should only be called on dead tracks, tracks are born dead
    
    void update(const bool matched, const float a = 0, const float f = 0, const float p = 0);//should not be called on dead tracks

    const bool isActive(void) const;
};




#endif  // TRACK_H_INCLUDED
