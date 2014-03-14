/*
  ==============================================================================

    SinusoidalModel.h
    Created: 11 Mar 2014 4:50:42pm
    Author:  Owen Campbell

  ==============================================================================
*/

#ifndef SINUSOIDALMODEL_H_INCLUDED
#define SINUSOIDALMODEL_H_INCLUDED

#include "Analysis.h"
#include "Oscillator.h"
#include <vector>

class Track{
public:
    enum class STATUS{ALIVE, DEAD, LIMBO};
private:
    float amp, frq, phs, lastAmp, lastFrq, lastPhs;
    STATUS status;
    int limboFrames, deathFrames;
public:
    Track(){
        status = STATUS::DEAD;
    };
    ~Track();
    
    void init(const float a, const float f, const float p, const int df){
        lastAmp = amp = a, lastFrq = frq = f, lastPhs = phs = p;
        status = STATUS::ALIVE;
        limboFrames = 0;
        deathFrames = df;
    }
    
    void update(const bool matched, const float a, const float f, const float p){
        if(matched){//continuing track
            lastAmp = amp, lastFrq = frq, lastPhs = phs;
            amp = a, frq = f, phs = p;
            status = STATUS::ALIVE;
            limboFrames = 0;
        }
        else if(status == STATUS::LIMBO){//already in limbo
            limboFrames++;
            if(limboFrames > deathFrames){
                status = STATUS::DEAD;
                limboFrames = 0;
            }
        }
        else{//fresh limbo state
            status = STATUS::LIMBO;
            limboFrames = 1;
        }
    }
    
    const bool isActive(void) const{
        return (status==STATUS::DEAD)?false:true;
    }
};

class SinusoidalModel{
friend Track;
friend Analysis;
private:
    Analysis * analysis;
    Track * tracks;
    Oscillator<float> * oscillators;
    Wavetable<float> * wavetable;
    int windowSize, maxTracks;
    float trackMagThreshold, trackFrqThreshold;
    typedef struct Peak{
        int idx = 0;
        float frq = 0.0;
        float mag = 0.0;
        float phs = 0.0;
    } Peak;
    std::vector<Peak> peaks;
    
public:
    SinusoidalModel(){
        analysis = new Analysis();
        tracks = new Track[0];
        oscillators = new Oscillator<float>[0];
        wavetable = new Wavetable<float>();
    }
    ~SinusoidalModel(){
        delete analysis;
        delete[] tracks;
        delete[] oscillators;
        delete wavetable;
    }
    
    void init(const Analysis::WINDOW w, const int ws, const float sr, const bool p,
              const int mt, const int wts, Wavetable<float>::WAVEFORM wf){
        windowSize = ws;
        maxTracks = mt;
        analysis->init(w, ws, sr, p);
        delete[] tracks;
        tracks = new Track[maxTracks];
        delete[] oscillators;
        oscillators = new Oscillator<float>[maxTracks];
        delete wavetable;
        wavetable = new Wavetable<float>(wf, wts);
    }
    void breakPoint(){
    }
    
    void interpolatePeak(const float x1, const float x2, const float x3,
                         const float y1, const float y2, const float y3, float &pX, float &pY){
        //adapted from http://stackoverflow.com/a/717791
        float denom = (x1 - x2) * (x1 - x3) * (x2 - x3);
        float a     = (x3 * (y2 - y1) + x2 * (y1 - y3) + x1 * (y3 - y2)) / denom;
        float b     = (x3*x3 * (y1 - y2) + x2*x2 * (y3 - y1) + x1*x1 * (y2 - y3)) / denom;
        float c     = (x2 * x3 * (x2 - x3) * y1 + x3 * x1 * (x3 - x1) * y2 + x1 * x2 * (x1 - x2) * y3) / denom;
        
        pX = -b / (2 * a);
        pY = c - b * b / (4 * a);
    }
    
    
    //getters
    float * getAnalysisResults(const Analysis::PARAMETER p) const{
        switch (p) {
            case Analysis::PARAMETER::MAG:
                return &analysis->getMagnitudes();
            case Analysis::PARAMETER::PHS:
                return &analysis->getPhases();
            case Analysis::PARAMETER::FRQ:
                return &analysis->getFrequencies();
            default:
                std::cout << "Error: attempting to retrieve analysis results with invalid parameter" << std::endl;
                return nullptr;
        }
    }
    
    void getLocalMaxima(){
        float * magnitudes = &analysis->getMagnitudes();
        float * phases = &analysis->getPhases();
        float * frequencies = &analysis->getFrequencies();
        float mag, magL, magR, phs, phsL, phsR, frq, frqL, frqR, peakMag, peakPhs, peakFrq;
        peaks.clear();
        for(int i = 1; i < windowSize - 1 && peaks.size() <= maxTracks; ++i){
            trackMagThreshold = 1.0 / log(frequencies[i]); //scaling threshold with frequency
            magL = magnitudes[i-1];
            mag = magnitudes[i];
            magR = magnitudes[i+1];
            if((mag > trackMagThreshold) && (magL < mag < magR)){
                //at local max
                frqL = frequencies[i-1];
                frq = frequencies[i];
                frqR = frequencies[i+1];
                phsL = phases[i-1];
                phs = phases[i];
                phsR = phases[i+1];
            
                //quadratically interpolate mag/frq
                interpolatePeak(magL, mag, magR, frqL, frq, frqR, peakMag, peakFrq);
                //linearly interpolate phase based on adjusted peak
                peakPhs = (peakFrq == frq)?phs:(peakFrq < frq)? (mag - magL)/(frq - frqL) * (peakFrq - frqL):
                                                                (magR - mag)/(frqR - frq) * (frqR - peakFrq);
                std::cout << "Peak detected. Frq: " << peakFrq << " Mag: " << peakMag << " Phs: " << peakPhs << std::endl;
                //peaks.push_back({PeakFrq, peakMag, peakPhs});
            }
        }
    }
    
    
                                
    
    

    //setters
    void setWaveform(Wavetable<float>::WAVEFORM wf){
        wavetable->setWaveform(wf, false);
    }
};



#endif  // SINUSOIDALMODEL_H_INCLUDED
