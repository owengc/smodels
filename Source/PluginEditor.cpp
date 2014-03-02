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
    addAndMakeVisible (graph = new Component());
    graph->setName ("Plot");


    //[UserPreSize]
    //[/UserPreSize]

    setSize (1024, 512);


    //[Constructor] You can add your own custom stuff here..
    startTimer(200);
    //[/Constructor]
}

SmodelsAudioProcessorEditor::~SmodelsAudioProcessorEditor()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
    //[/Destructor_pre]

    graph = nullptr;


    //[Destructor]. You can add your own custom destruction code here..
    //[/Destructor]
}

//==============================================================================
void SmodelsAudioProcessorEditor::paint (Graphics& g)
{
    //[UserPrePaint] Add your own custom painting code here..
    //[/UserPrePaint]

    g.fillAll (Colour (0xff3f474e));

    g.setColour (Colours::grey);
    g.fillRect (456, 24, 544, 288);

    //[UserPaint] Add your own custom painting code here..
    g.setColour(Colour(255, 0, 0));
    int i=0, graphRes = getGraphResolution();
    float graphLeft = (float)graph->getPosition().getX(), graphWidth = (float)graph->getWidth(),
        graphTop = (float)graph->getPosition().getY(), graphHeight = (float)graph->getHeight(),
        rectWidth = graphWidth / graphRes, topOfBar, mag;
    for(; i<graphRes; ++i){
        mag = ((float)i / graphRes);
        topOfBar = graphTop + (graphHeight - (graphHeight * mag));
        //graphOrigin.getX()+(i * rectWidth), graphOrigin.getY(), rectWidth, graphHeight * (float)graphWidth / i
        g.drawRect(graphLeft+(i * rectWidth), topOfBar, rectWidth, mag * graphHeight);
        
        //(float)graph->getPosition().getY() + graphHeight * ((float)i / graphRes)
    }
        //    }
    /*  WDL_COMPLEX * mags = AudioProcessor.mags
    g.drawVerticalLine(window_origin_x+i, mags[i], float(window_origin_y))*/
    //[/UserPaint]
}

void SmodelsAudioProcessorEditor::resized()
{
    graph->setBounds (proportionOfWidth (0.4609f), proportionOfHeight (0.0781f), proportionOfWidth (0.4971f), proportionOfHeight (0.5020f));
    //[UserResized] Add your own custom resize handling here..
    //[/UserResized]
}



//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...
void SmodelsAudioProcessorEditor::timerCallback(){
    SmodelsAudioProcessor* ourProcessor = getProcessor();
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
  <GENERICCOMPONENT name="Plot" id="33568b08c10d6ccd" memberName="graph" virtualName=""
                    explicitFocusOrder="0" pos="46.094% 7.812% 49.707% 50.195%" class="Component"
                    params=""/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif


//[EndFile] You can add extra defines here...
//[/EndFile]
