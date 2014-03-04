/*
  ==============================================================================

    Analysis.cpp
    Created: 1 Mar 2014 10:17:35pm
    Author:  Owen Campbell

  ==============================================================================
*/

#include "Analysis.h"

//getters
float Analysis::get(const int index, const PARAMETER p) const{
    switch(p){
        case REAL:
            return getReal(index);
        case IMAG:
            return getImag(index);
        case MAG:
            return getMag(index);
        case PHS:
            return getPhs(index);
        case FRQ:
            return getFrq(index);
        default:
            std::cout << "Analysis.get called with invalid parameter" << std::endl;
            return 0.0f;
    }
}
float Analysis::getReal(const int index) const{
    try{
        return complexBuffer[index].re;
    }
    catch(std::exception){
        std::cout << "Attempting to access out of range real number index." << std::endl;
        return 0.0f;
    }
}
float Analysis::getImag(const int index) const{
    try{
        return complexBuffer[index].im;
    }
    catch(std::exception){
        std::cout << "Attempting to access out of range imaginary number index." << std::endl;
        return 0.0f;
    }
}
float Analysis::getMag(const int index) const{
    try{
        return magnitudes[index];
    }
    catch(std::exception){
        std::cout << "Attempting to access out of range magnitude index." << std::endl;
        return 0.0f;
    }
}
float Analysis::getPhs(const int index) const{
    try{
        return phases[index];
    }
    catch(std::exception){
        std::cout << "Attempting to access out of range phs index." << std::endl;
        return 0.0f;
    }
}
float Analysis::getFrq(const int index) const{
    try{
        return frequencies[index];
    }
    catch(std::exception){
        std::cout << "Attempting to access out of range frequency index." << std::endl;
        return 0.0f;
    }
}

//setters
void setComplex(const int index, const float realVal, const float imagVal = 0.0f){
    try{
        complexBuffer[index].re = realVal;
        complexBuffer[index].im = imagVal;
    }
    catch(std::exception){
        std::cout << "Attempting to set out of range complex number index." << std::endl;
    }
}
void Analysis::set(const int index, const PARAMETER p, const float val){
    switch(p){
        case REAL:
            return setReal(index, val);
        case IMAG:
            return setImag(index, val);
        case MAG:
            return setMag(index, val);
        case PHS:
            return setPhs(index, val);
        default:
            std::cout << "Analysis.set called with invalid parameter" << std::endl;
    }
}
void Analysis::setReal(const int index, const float val){
    try{
        complexBuffer[index].re = val;
    }
    catch(std::exception){
        std::cout << "Attempting to set out of range real number index." << std::endl;
    }
}
void Analysis::setImag(const int index, const float val){
    try{
        complexBuffer[index].im = val;
    }
    catch(std::exception){
        std::cout << "Attempting to set out of range imaginary number index." << std::endl;
    }
}
void Analysis::setMag(const int index, const float val){
    try{
        magnitudes[index] = val;
    }
    catch(std::exception){
        std::cout << "Attempting to set out of range magnitude index." << std::endl;
    }
}
void Analysis::setPhs(const int index, const float val){
    try{
        magnitudes[index] = val;
    }
    catch(std::exception){
        std::cout << "Attempting to set out of range magnitude index." << std::endl;
    }
}
