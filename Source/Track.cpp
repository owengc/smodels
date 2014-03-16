/*
  ==============================================================================

    Track.cpp
    Created: 15 Mar 2014 5:34:50pm
    Author:  Owen Campbell

  ==============================================================================
*/

#include "Track.h"
#include "SinusoidalModel.h"

void Track::init(SinusoidalModel * m, const float a, const float f, const float p){//should only be called on dead tracks, tracks are born dead
    lastAmp = amp = a, lastFrq = frq = f, lastPhs = phs = p;
    status = STATUS::ALIVE;
    limboFrames = 0;
    model = m;//store pointer to parent model
    model->activeTracks++;
}

void Track::update(const bool matched, const float a, const float f, const float p){//should not be called on dead tracks
    if(matched){//continuing track
        lastAmp = amp, lastFrq = frq, lastPhs = phs;
        amp = a, frq = f, phs = p;
        status = STATUS::ALIVE;
        limboFrames = 0;
    }
    else{
        if(status == STATUS::LIMBO){//already in limbo
            limboFrames++;
            if(limboFrames == model->trackDeath){
                status = STATUS::DEAD;
                model->activeTracks--;
            }
        }
        else{//fresh limbo state
            status = STATUS::LIMBO;
            limboFrames = 1;
        }
    }
}

const bool Track::isActive(void) const{
    return (status==STATUS::DEAD)?false:true;
}
