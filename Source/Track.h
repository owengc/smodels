/*
  ==============================================================================

    Track.h
    Created: 15 Mar 2014 5:34:50pm
    Author:  Owen Campbell

  ==============================================================================
*/

#ifndef TRACK_H_INCLUDED
#define TRACK_H_INCLUDED
#include <cmath>

class SinusoidalModel;

class Track{
friend class SinusoidalModel;
public:
    enum class STATUS{BIRTH, ALIVE, DYING, DEAD};
private:
    float amp, frq, phs;
    STATUS status;
    int aliveFrames, birthFrames, dyingFrames;
    SinusoidalModel * model;
public:
    bool active;
    Track(){
        status = STATUS::DEAD;
        active = false;
        model = nullptr;
    };
    ~Track(){
        model = nullptr;
    };
    
    void init(SinusoidalModel * m/*, const float a, const float f, const float p*/);//should only be called on dead tracks, tracks are born dead
    
    void update(const bool matched, const float a = 0, const float f = 0, const float p = 0);//should not be called on dead tracks

    //const bool isActive(void) const;
    const bool isDead(void) const;
};

class TrackMatch{
//helper class for matching peaks to tracks
public:
	int idx;
	bool detected, assigned;
	float distSq, amp, frq, phs, ampDiff, frqDiff, phsDiff;
	TrackMatch(const int i = -1, const float a = 0, const float f = 0, const float p = 0){
		reset();
		init(i, false, a, f, p);
	}
	void init(const int i, const bool d, const float a, const float f, const float p){
		idx = i;
		detected = d;
		amp = a;
		frq = f;
		phs = p;
	}
	void reset(){
		idx = -1;
		amp = frq = phs = -MAXFLOAT;
		ampDiff = frqDiff = phsDiff = distSq = MAXFLOAT;
		detected = assigned = false;
	}
	void setDistanceSq(const float a, const float f, const float p){
		ampDiff = amp - a;
		frqDiff = fabs(frq - f);//need abs for comparisons
		phsDiff = phs - p;
		distSq = ampDiff * ampDiff + frqDiff * frqDiff + phsDiff * phsDiff;
	}
};


#endif  // TRACK_H_INCLUDED
