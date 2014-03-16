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
    UIUpdateFlag = true;
    SpectrogramUpdateFlag = true;
    analysisSize = 1024;
    zeroPadding = true;
    //analyses = new Analysis[0];
    smodels = new SinusoidalModel[0];
    //std::cout << "processor constructor loc: " << this << std::endl;
}
    

SmodelsAudioProcessor::~SmodelsAudioProcessor()
{
    //delete[] analyses;
    delete[] smodels;
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
    int numChannels = getNumInputChannels();
    //delete[] analyses;
    //analyses = new Analysis[numChannels];
    delete[] smodels;
    smodels = new SinusoidalModel[numChannels];
    for(int i = 0; i < numChannels; ++i){
        //analyses[i].init(Analysis::WINDOW::HANN, analysisSize, (float)sampleRate, zeroPadding);
        smodels[i].init(Analysis::WINDOW::HANN, analysisSize, (float)sampleRate, zeroPadding, 4096, Wavetable<float>::WAVEFORM::SINE);
    }
    //std::cout << "processor prepareToPlay loc: " << this << std::endl;
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
    
    
    int numChannels = buffer.getNumChannels(), numSamples = buffer.getNumSamples(), channel = 0, index;
    //std::cout << "Callback size: " << callbackSize << std::endl;
    //TODO: instead of monoBuffering, maybe I should try one of those stereo optimizations (treat the two channels as one complex number and take the complex fft, for example)
    float sample;
    bool update = false;
    for (; channel < numChannels; ++channel){
        float * channelData = buffer.getSampleData(channel);
        for (index = 0; index < numSamples; ++index){
            sample = channelData[index];
            //if(analyses[channel](sample)){//write values to analysis buffer
            if(smodels[channel](sample)){//write values to analysis buffer
                //take an fft now!
                //analyses[channel].transform(Analysis::TRANSFORM::FFT);
                //analyses[channel].transform(Analysis::TRANSFORM::IFFT);
                smodels[channel].transform(Analysis::TRANSFORM::FFT);
                smodels[channel].breakpoint();
                smodels[channel].transform(Analysis::TRANSFORM::IFFT);
                update = true;
            }
            else{
                update = false;
            }
        }
    }
    SpectrogramUpdateFlag = update?true:false;
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

int SmodelsAudioProcessor::getAnalysisSize() const{
    return (zeroPadding)?analysisSize * 2:analysisSize;
}

float * SmodelsAudioProcessor::getAnalysisResults(const int channel, const Analysis::PARAMETER p) const{
   /* switch (p) {
        case Analysis::PARAMETER::MAG:
            return &analyses[channel].getMagnitudes();
        case Analysis::PARAMETER::PHS:
            return &analyses[channel].getPhases();
        case Analysis::PARAMETER::FRQ:
            return &analyses[channel].getFrequencies();
        default:
            std::cout << "Error: attempting to retrieve analysis results with invalid parameter" << std::endl;
            return nullptr;
    }*/
    return smodels[channel].getAnalysisResults(p);
}


//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SmodelsAudioProcessor();
}
