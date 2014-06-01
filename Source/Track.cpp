/*
  ==============================================================================

    Track.cpp
    Created: 15 Mar 2014 5:34:50pm
    Author:  Owen Campbell

  ==============================================================================
*/

#include "Track.h"
#include "SinusoidalModel.h"


void Track::init(SinusoidalModel * m, const float a, const float f, const float p){//should only be called on dead tracks
    assert(status == STATUS::DEAD);
    amp = a, frq = f, phs = p;
    status = STATUS::BIRTH;
    aliveFrames = 0;
    birthFrames = 0;
    dyingFrames = 0;
    model = m;//store pointer to parent model
    active = false;
}


void Track::update(const bool matched, const float a, const float f, const float p){//should never be called on dead tracks
    assert(status != STATUS::DEAD);
    if(matched){//continuing track
        amp = a, frq = f, phs = p;
        if(status == STATUS::BIRTH){//birthing
            birthFrames++;
            if(birthFrames >= model->trackBirth){
                status = STATUS::ALIVE;
            }
        }
        else if(status == STATUS::DYING){//revived
            status = STATUS::ALIVE;
            dyingFrames = 0;
        }
        aliveFrames++;
		if(aliveFrames > model->longestTrack){
			model->longestTrack = aliveFrames;
		}
    }
    else{//in limbo
        if(status == STATUS::BIRTH){//birth failed
            status = STATUS::DEAD;
        }
        else if(status == STATUS::DYING){//condition not improving
            dyingFrames++;
            if(dyingFrames >= model->trackDeath){//it's bleedin' demised
                status = STATUS::DEAD;
                aliveFrames = 0;
            }
        }
        else if(status == STATUS::ALIVE){//initial signs of decay
            status = STATUS::DYING;
        }
    }
    //using 'active' as an optimization to reduce comparisons
    if(status == STATUS::BIRTH || status == STATUS::DEAD){
        active = false;
    }
    else{
        active = true;
    }
}

//a track is active if it is alive or dying. track is not active if it is dead or in birth
/*const bool Track::isActive(void) const{
    return active;
}*/
const bool Track::isDead(void) const{
    return (status == STATUS::DEAD)?true:false;
}