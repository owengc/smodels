/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic startup code for a Juce application.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"


//==============================================================================
SmodelsAudioProcessor::SmodelsAudioProcessor()
{
}

SmodelsAudioProcessor::~SmodelsAudioProcessor()
{
}

//==============================================================================
const String SmodelsAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

int SmodelsAudioProcessor::getNumParameters()
{
    return 0;
}

float SmodelsAudioProcessor::getParameter (int index)
{
    return 0.0f;
}

void SmodelsAudioProcessor::setParameter (int index, float newValue)
{
}

const String SmodelsAudioProcessor::getParameterName (int index)
{
    return String::empty;
}

const String SmodelsAudioProcessor::getParameterText (int index)
{
    return String::empty;
}

const String SmodelsAudioProcessor::getInputChannelName (int channelIndex) const
{
    return String (channelIndex + 1);
}

const String SmodelsAudioProcessor::getOutputChannelName (int channelIndex) const
{
    return String (channelIndex + 1);
}

bool SmodelsAudioProcessor::isInputChannelStereoPair (int index) const
{
    return true;
}

bool SmodelsAudioProcessor::isOutputChannelStereoPair (int index) const
{
    return true;
}

bool SmodelsAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool SmodelsAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool SmodelsAudioProcessor::silenceInProducesSilenceOut() const
{
    return false;
}

double SmodelsAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int SmodelsAudioProcessor::getNumPrograms()
{
    return 0;
}

int SmodelsAudioProcessor::getCurrentProgram()
{
    return 0;
}

void SmodelsAudioProcessor::setCurrentProgram (int index)
{
}

const String SmodelsAudioProcessor::getProgramName (int index)
{
    return String::empty;
}

void SmodelsAudioProcessor::changeProgramName (int index, const String& newName)
{
}

//==============================================================================
void SmodelsAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
}

void SmodelsAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

void SmodelsAudioProcessor::processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages)
{
    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    for (int channel = 0; channel < getNumInputChannels(); ++channel)
    {
        float* channelData = buffer.getSampleData (channel);

        // ..do something to the data...
    }

    // In case we have more outputs than inputs, we'll clear any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    for (int i = getNumInputChannels(); i < getNumOutputChannels(); ++i)
    {
        buffer.clear (i, 0, buffer.getNumSamples());
    }
}

//==============================================================================
bool SmodelsAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* SmodelsAudioProcessor::createEditor()
{
    return new SmodelsAudioProcessorEditor (this);
}

//==============================================================================
void SmodelsAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void SmodelsAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SmodelsAudioProcessor();
}
