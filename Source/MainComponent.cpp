/*
  MainContentComponent for AnymaPal
*/

#ifndef MAINCOMPONENT_H_INCLUDED
#define MAINCOMPONENT_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"

#include "MidiProcessor.cpp"

class MainContentComponent
  : public juce::Component
  , private juce::Button::Listener
{
private:
    ScopedPointer<juce::LookAndFeel_V1> uiLookAndFeel;
  
    juce::ImageComponent uiImage_background;
    juce::TextButton uiTextButton_toggle;
    juce::Label uiLabel_status;
    juce::Label uiLabel_info;

    ScopedPointer<MidiProcessorComponent> midiProc;
  
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainContentComponent)
  
public:
    MainContentComponent()
    {
        // flat style is in, flat style is out
        // all just little bits of history repeating :)
        uiLookAndFeel = new LookAndFeel_V1();
        setLookAndFeel( uiLookAndFeel );
        uiLookAndFeel->setColour(
          juce::Label::ColourIds::textColourId,
          juce::Colours::lightgrey );
    
        addAndMakeVisible( uiLabel_status );
        uiRefreshStatus();
      
        addAndMakeVisible( uiTextButton_toggle );
        uiTextButton_toggle.addListener( this ); // buttonClicked
        // setSize() below sets up call chain which calls uiRefreshStatus() which calls setButtonText() and setColor()
      
        // set up background image
        PNGImageFormat loader;
        File imgFile = File::getSpecialLocation(File::SpecialLocationType::currentApplicationFile);
        imgFile = imgFile.getChildFile( "Contents/Resources/anymaPal.png" );
        uiImage_background.setImage(
          loader.loadFrom( imgFile ),
          RectanglePlacement::fillDestination );
        addAndMakeVisible( uiImage_background );

        // set up midi processor
        midiProc = new MidiProcessorComponent( uiLabel_status );
        addAndMakeVisible( midiProc );
    
        // refresh available ports and auto-select port 0
        // -1 means "no preferred device index" 
        midiProc->uiRefreshMidiInputList( -1, "Anyma Phi" );
        midiProc->uiRefreshMidiOutputList( -1, "Anyma Phi" );
      
        repaint();
        setSize( 400, 300 ); //calls resized() to position objects
    }

    ~MainContentComponent()
    {
      
    }
  
    //================================================================
#pragma mark ui callbacks
    void buttonClicked(Button* buttonThatWasClicked) override
    {
        if( buttonThatWasClicked == &uiTextButton_toggle)
        {
            if( nullptr == midiProc ) return;
            midiProc->toggle();
            uiRefreshStatus();
        }
    }
  
    //================================================================
#pragma mark ui related
    void uiRefreshStatus()
    {
      // apply default state
      uiLabel_status.setEditable(false);
      uiLabel_status.setText("Loading", dontSendNotification);
      uiLabel_status.setColour(juce::Label::ColourIds::backgroundColourId, Colours::transparentBlack);
      uiLabel_status.setColour(juce::Label::ColourIds::backgroundWhenEditingColourId, Colours::lightgrey);
      uiLabel_status.setColour(juce::Label::ColourIds::textColourId, Colours::grey);
      uiLabel_status.setColour(juce::Label::ColourIds::textWhenEditingColourId, Colours::white);
      
      juce::Colour uiColour_barelyBlue = Colour(0xffccffff);
      
      if( nullptr == midiProc ) return;
      bool isActive = midiProc->isActive();
      
      if(isActive)
      {
        uiLabel_status.setText("Active", dontSendNotification);
        
        uiTextButton_toggle.setButtonText("Disable");
        uiTextButton_toggle.setColour(TextButton::ColourIds::buttonColourId, Colours::grey);
        uiTextButton_toggle.setColour(TextButton::ColourIds::textColourOffId, uiColour_barelyBlue);
      }
      else
      {
        uiLabel_status.setText("Idle", dontSendNotification);
       
        uiTextButton_toggle.setButtonText("Activate");
        uiTextButton_toggle.setColour(TextButton::ColourIds::buttonColourId, uiColour_barelyBlue);
        uiTextButton_toggle.setColour(TextButton::ColourIds::textColourOffId, Colours::grey);
      }
    }
  
    //=======================================================================
#pragma ui component level
    void paint (Graphics& g) override
    {
        g.fillAll (Colours::black);
    }

    void resized() override
    {
        uiRefreshStatus();
    
        auto initialArea = getLocalBounds();
      
        Rectangle<int> area = initialArea;
        uiImage_background.setBounds( area );
        uiImage_background.toBack();
      
        {
          Rectangle<int> area = initialArea;
          midiProc->setBounds( area.withTrimmedBottom( 36 ) );
        }
      
        // horizontally split half
        {
          Rectangle<int> area = initialArea;
          area = area.withTop( midiProc->getBounds().getBottom() );
          
          uiLabel_status.setBounds(area.reduced(4).withWidth( area.getWidth() / 2));
          uiTextButton_toggle.setBounds(area.reduced(4).withLeft( area.getWidth() / 2 ));
        }
      
    }
};

// createMainContentComponent() called by main
Component* createMainContentComponent()     { return new MainContentComponent(); }

#endif  // MAINCOMPONENT_H_INCLUDED
