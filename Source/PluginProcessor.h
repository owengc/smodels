/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic startup code for a Juce application.

  ==============================================================================
*/

#ifndef PLUGINPROCESSOR_H_INCLUDED
#define PLUGINPROCESSOR_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"
#include "Analysis.h"

//==============================================================================
/**
*/
class SmodelsAudioProcessor  : public AudioProcessor
{
public:
    //==============================================================================
    SmodelsAudioProcessor();
    ~SmodelsAudioProcessor();

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock);
    void releaseResources();

    void processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages);

    //==============================================================================
    AudioProcessorEditor* createEditor();
    bool hasEditor() const;

    //==============================================================================
    const String getName() const;

    int getNumParameters();

    float getParameter (int index);
    void setParameter (int index, float newValue);

    const String getParameterName (int index);
    const String getParameterText (int index);

    const String getInputChannelName (int channelIndex) const;
    const String getOutputChannelName (int channelIndex) const;
    bool isInputChannelStereoPair (int index) const;
    bool isOutputChannelStereoPair (int index) const;

    bool acceptsMidi() const;
    bool producesMidi() const;
    bool silenceInProducesSilenceOut() const;
    double getTailLengthSeconds() const;

    //==============================================================================
    int getNumPrograms();
    int getCurrentProgram();
    void setCurrentProgram (int index);
    const String getProgramName (int index);
    void changeProgramName (int index, const String& newName);

    //==============================================================================
    void getStateInformation (MemoryBlock& destData);
    void setStateInformation (const void* data, int sizeInBytes);

    //Custom Methods, Params, and Public Data
    int getAnalysisSize(const int range) const;
    float getAnalysisValue(const int index, const int dim) const;
    /*enum Parameters{
        MasterBypass = 0,
        Mix,
        Decay,
        CombDelay1,
        CombDelay2,
        CombDelay3,
        CombDelay4,
        AllpassGain1,
        AllpassDelay1,
        AllpassGain2,
        AllpassDelay2,
        LowpassCutoff,
        NumParams
    };*/
    bool NeedsUIUpdate(){return UIUpdateFlag;};
    void ClearUIUpdateFlag(){UIUpdateFlag = false;};
    void RaiseUIUpdateFlag(){UIUpdateFlag = true;};
    WDL_FFT_COMPLEX * UIAnalysisCache;
    
private:
    //Private Data, helper methods, etc
    int blockSize;
    float * monoBuffer;
    float scaleFactor;
    Analysis * analyses;
    bool UIUpdateFlag;
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SmodelsAudioProcessor)
};

#endif  // PLUGINPROCESSOR_H_INCLUDED