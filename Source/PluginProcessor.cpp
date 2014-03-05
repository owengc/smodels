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
    //UIUpdateFlag = true;
    int sr = getSampleRate();
    int numChannels = getNumInputChannels();
    int windowSize = 1024;
    analyses = new Analysis[numChannels];
    UIAnalysisCache = new float * [numChannels * 2]; //probs a better way to do this
    for(int i = 0; i < numChannels; ++i){
        analyses[i].init();
        analyses[i].resize(windowSize, sr);
        UIAnalysisCache[2 * i] = &analyses[i].getFrequencies();
        UIAnalysisCache[2 * i + 1] = &analyses[i].getMagnitudes();
    }
}
    

SmodelsAudioProcessor::~SmodelsAudioProcessor()
{
    delete[] analyses;
    delete[] UIAnalysisCache;
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
    
    int blockSize = getBlockSize();
    std::cout << "Block size: " << blockSize << std::endl;
    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    
    
    int numChannels = buffer.getNumChannels(), numSamples = buffer.getNumSamples(), channel = 0, sample = 0;
    int callbackSize = getBlockSize(), frameCount = 0, timer = 0;
    std::cout << "Callback size: " << callbackSize << std::endl;
    //TODO: instead of monoBuffering, maybe I should try one of those stereo optimizations (treat the two channels as one complex number and take the complex fft, for example)
    for (; channel < 1/*numChannels*/; ++channel){
        float* channelData = buffer.getSampleData(channel);
        for (; sample < numSamples; ++sample){
            if(analyses[channel](channelData[sample])){
                //take an fft now!
                std::cout << "fft frame " << frameCount << " ready (" << timer << ")" << std::endl;
                timer = 0;
            }
            timer++;
        }
    }
    // In case we have more outputs than inputs, we'll clear any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    for (int i = getNumInputChannels(); i < getNumOutputChannels(); ++i)
    {
        buffer.clear(i, 0, buffer.getNumSamples());
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

int SmodelsAudioProcessor::getAnalysisSize(const int channel) const{
    return analyses[channel].getWindowSize() / 2 + 1;
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SmodelsAudioProcessor();
}
