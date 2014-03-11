/*
  ==============================================================================

  This is an automatically generated GUI class created by the Introjucer!

  Be careful when adding custom code to these files, as only the code within
  the "//[xyz]" and "//[/xyz]" sections will be retained when the file is loaded
  and re-saved.

  Created with Introjucer version: 3.1.0

  ------------------------------------------------------------------------------

  The Introjucer is part of the JUCE library - "Jules' Utility Class Extensions"
  Copyright 2004-13 by Raw Material Software Ltd.

  ==============================================================================
*/

//[Headers] You can add your own extra header files here...
//[/Headers]

#include "PluginEditor.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...
//[/MiscUserDefs]

//==============================================================================
SmodelsAudioProcessorEditor::SmodelsAudioProcessorEditor (SmodelsAudioProcessor* ownerFilter)
    : AudioProcessorEditor(ownerFilter)
{
    addAndMakeVisible (screen = new Component());
    screen->setName ("Screen");

    //[UserPreSize]
    //[/UserPreSize]

    setSize (1024, 512);

    //[Constructor] You can add your own custom stuff here..
    addAndMakeVisible(spectrogram = new Spectrogram(ownerFilter, screen->getBounds()));
    spectrogram->setName("Spectrogram");
    spectrogram->toFront(false);
    startTimer(200);
    //[/Constructor]
}

SmodelsAudioProcessorEditor::~SmodelsAudioProcessorEditor()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
    //[/Destructor_pre]

    screen = nullptr;


    //[Destructor]. You can add your own custom destruction code here..
    //[/Destructor]
}

//==============================================================================
void SmodelsAudioProcessorEditor::paint (Graphics& g)
{
    //[UserPrePaint] Add your own custom painting code here..
    //[/UserPrePaint]
    //std::cout << "attempting to paint UI" << std::endl;
    g.fillAll (Colour (0xff3f474e));

    g.setColour (Colours::grey);
    g.fillRect (456, 24, 544, 288);

    //[UserPaint] Add your own custom painting code here..
    //[/UserPaint]
}

void SmodelsAudioProcessorEditor::resized()
{
    screen->setBounds (proportionOfWidth (0.4609f), proportionOfHeight (0.0781f), proportionOfWidth (0.4971f), proportionOfHeight (0.5020f));
    //[UserResized] Add your own custom resize handling here..
    //[/UserResized]
}



//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...
void SmodelsAudioProcessorEditor::timerCallback(){
    //std::cout << "UI timer called" << std::endl;
    SmodelsAudioProcessor* ourProcessor = getProcessor();
    if(ourProcessor->NeedsUIUpdate()){
        ourProcessor->ClearUIUpdateFlag();
    }
    /*
     //exchange any data you want between UI elements and the plugin "ourProcessor"
     if(ourProcessor->NeedsUIUpdate()){
     BypassButton->setToggleState(1.0f == ourProcessor->getParameter(XVerbAudioProcessor::MasterBypass), false);
     MixKnob->setValue(ourProcessor->getParameter(XVerbAudioProcessor::Mix), juce::dontSendNotification);
     DecayKnob->setValue(ourProcessor->getParameter(XVerbAudioProcessor::Mix), juce::dontSendNotification);
     CombDelayKnob1->setValue(ourProcessor->getParameter(XVerbAudioProcessor::CombDelay1), juce::dontSendNotification);
     CombDelayKnob2->setValue(ourProcessor->getParameter(XVerbAudioProcessor::CombDelay2), juce::dontSendNotification);
     CombDelayKnob3->setValue(ourProcessor->getParameter(XVerbAudioProcessor::CombDelay3), juce::dontSendNotification);
     CombDelayKnob4->setValue(ourProcessor->getParameter(XVerbAudioProcessor::CombDelay4), juce::dontSendNotification);
     AllpassDelayKnob1->setValue(ourProcessor->getParameter(XVerbAudioProcessor::AllpassDelay1), juce::dontSendNotification);
     AllpassGainKnob1->setValue(ourProcessor->getParameter(XVerbAudioProcessor::AllpassGain1), juce::dontSendNotification);
     AllpassDelayKnob2->setValue(ourProcessor->getParameter(XVerbAudioProcessor::AllpassDelay2), juce::dontSendNotification);
     AllpassGainKnob2->setValue(ourProcessor->getParameter(XVerbAudioProcessor::AllpassGain2), juce::dontSendNotification);
     LowpassCutoffKnob->setValue(ourProcessor->getParameter(XVerbAudioProcessor::LowpassCutoff), juce::dontSendNotification);
     ourProcessor->ClearUIUpdateFlag();
     //std::cout << "UIUpdated" << std::endl;
     }
     */
}
//[/MiscUserCode]


//==============================================================================
#if 0
/*  -- Introjucer information section --

    This is where the Introjucer stores the metadata that describe this GUI layout, so
    make changes in here at your peril!

BEGIN_JUCER_METADATA

<JUCER_COMPONENT documentType="Component" className="SmodelsAudioProcessorEditor"
                 componentName="" parentClasses="public AudioProcessorEditor, public Timer"
                 constructorParams="SmodelsAudioProcessor* ownerFilter" variableInitialisers="AudioProcessorEditor(ownerFilter)"
                 snapPixels="8" snapActive="1" snapShown="1" overlayOpacity="0.330"
                 fixedSize="1" initialWidth="1024" initialHeight="512">
  <BACKGROUND backgroundColour="ff3f474e">
    <RECT pos="456 24 544 288" fill="solid: ff808080" hasStroke="0"/>
  </BACKGROUND>
  <GENERICCOMPONENT name="Screen" id="33568b08c10d6ccd" memberName="screen" virtualName=""
                    explicitFocusOrder="0" pos="46.094% 7.812% 49.707% 50.195%" class="Component"
                    params=""/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif


//[EndFile] You can add extra defines here...
//[/EndFile]
