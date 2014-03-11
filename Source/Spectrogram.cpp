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

#include "Spectrogram.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...
//[/MiscUserDefs]

//==============================================================================
Spectrogram::Spectrogram (SmodelsAudioProcessor* ownerFilter, const Rectangle<int> b)
    : ourProcessor(ownerFilter), bounds(b)
{

    //[UserPreSize]
    //[/UserPreSize]

    setSize (512, 256);


    //[Constructor] You can add your own custom stuff here..
    //SmodelsAudioProcessor* ourProcessor = getProcessor();
    graphResolution = ourProcessor->getAnalysisSize() / 2 + 1;
    setBounds(bounds);
    //std::cout << "constructor spectro right bound: " << bounds.getRight() << std::endl;
    startTimer(20);
    //std::cout << "spectrogram constructor loc: " << ourProcessor << std::endl;
    //[/Constructor]
}

Spectrogram::~Spectrogram()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
    //[/Destructor_pre]



    //[Destructor]. You can add your own custom destruction code here..
    //[/Destructor]
}

//==============================================================================
void Spectrogram::paint (Graphics& g)
{
    //[UserPrePaint] Add your own custom painting code here..
    //[/UserPrePaint]

    g.fillAll (Colour (0xff1c4151));

    //[UserPaint] Add your own custom painting code here..
    bounds = g.getClipBounds();
    //std::cout << "attempting to paint spectrogram" << std::endl;
    int numChannels = ourProcessor->getNumInputChannels(), channel = 0, i;
    float graphWidth = bounds.getWidth(), graphLeft = 0.0,
    graphHeight = bounds.getHeight(), graphBottom = bounds.getBottom(), graphTop = 0.0,
    barWidth = graphWidth / graphResolution, barLeft, barTop, barHeight;
    float * magnitudes, * mag;
    for(; channel < numChannels; ++channel){
        magnitudes = ourProcessor->getAnalysisResults(channel, Analysis::PARAMETER::MAG);
        /*if(channel == 0){
            g.setColour(Colour(255, 0, 0, ));
        }
        else{
            g.setColour(Colour(0, 0, 255));
        }*/
        for(i = 0; i < graphResolution; ++i){
            barLeft = /*graphLeft+*/(i * barWidth);
            mag = &magnitudes[i];
            if(*mag > 0.01f){
                //std::cout << "spectro: mag greater than .1: " << *mag<< std::endl;
                barHeight = (*mag < 1.0f)?graphHeight * *mag:graphHeight; //clamp bar height to full for clipped magnitudes
                barTop = graphTop + (graphHeight - barHeight);
                //                g.drawRect(barLeft, barTop, barWidth, barHeight);

            
                //    g.drawLine(barLeft, barTop, barLeft, graphBottom, barWidth);
            }
            float alpha = 1.0;//0.2 + *mag;
                              //barTop = graphTop;
            barTop = graphHeight * (float)i/graphResolution;
            if(i < 200){
                g.setColour(Colour(0.75, 1.0, 0.5f, alpha));
            }
            else if(i >400){
               g.setColour(Colour(0.15, 1.0, alpha, alpha));//yellow
            }
            else{
                g.setColour(Colour(0.0, 0.0, 0.0f, 0.0f));
            }
            g.drawVerticalLine(barLeft, barTop, graphBottom);//  drawLine(barLeft, barTop, barLeft, graphBottom, barWidth);
        }
    }
    //[/UserPaint]
}

void Spectrogram::resized()
{
    //[UserResized] Add your own custom resize handling here..
    //[/UserResized]
}



//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...
void Spectrogram::timerCallback()
{
    //std::cout << "spectrogram timer called" << std::endl;
    if(ourProcessor->NeedsSpectrogramUpdate()){
        repaint();
        ourProcessor->ClearSpectrogramUpdateFlag();
    }
};
//[/MiscUserCode]


//==============================================================================
#if 0
/*  -- Introjucer information section --

    This is where the Introjucer stores the metadata that describe this GUI layout, so
    make changes in here at your peril!

BEGIN_JUCER_METADATA

<JUCER_COMPONENT documentType="Component" className="Spectrogram" componentName=""
                 parentClasses="public Component, public Timer" constructorParams="SmodelsAudioProcessor* ownerFilter, const Rectangle&lt;int&gt; b"
                 variableInitialisers="ourProcessor(ownerFilter), bounds(b)" snapPixels="8"
                 snapActive="1" snapShown="1" overlayOpacity="0.330" fixedSize="1"
                 initialWidth="512" initialHeight="256">
  <BACKGROUND backgroundColour="ff1c4151"/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif


//[EndFile] You can add extra defines here...
//[/EndFile]