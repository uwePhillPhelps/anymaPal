/*
  MidiProcessor          - logic, MIDI tx and rx
  MidiProcessorComponent - juce user interface
*/

#include "../JuceLibraryCode/JuceHeader.h"

#include "SyxRepeater.h"

class MainContentComponent; // fwd declaration

class MidiProcessor
      : private juce::MidiInputCallback
{
private:
  // system exclusive messages to anyma hardware
  const uint8_t keepAliveSyx[3] = { 0xF0, 0x71, 0xF7 }; // 'q'
  const uint8_t getStatusSyx[5] = { 0xF0, 0x71, 0x62, 0x06, 0xF7 }; // 'qb' 6
  const uint8_t eModeSyx[7] = { 0xF0, 0x00, 0x21, 0x33, 0x71, 0x00, 0xF7 }; // 0 '!3q' 0
  const uint8_t patchSyx[7] = { 0xF0, 0x00, 0x21, 0x33, 0x71, 0x11, 0xF7 }; // 0 '!3q' 11

  // regularly TX to anyma hardware
  SyxRepeater anymaKeepAlive;
  SyxRepeater anymaGetStatus;
  
  int fromAnymaIndex = 0; // index of juce::MidiInput device
  juce::AudioDeviceManager deviceManager;

  juce::ScopedPointer<juce::MidiOutput> midiToSequencer;
  juce::ScopedPointer<juce::MidiOutput> midiToAnyma;

public:
  MidiProcessor()
  {
    anymaKeepAlive.setMsg( keepAliveSyx, 3 );
    anymaKeepAlive.setInterval( 1000 );
    
    anymaGetStatus.setMsg( getStatusSyx, 5 );
    anymaGetStatus.setInterval( 200 );
  }
  
  ~MidiProcessor()
  {
  }
  
  void start()
  {
    // request the anyma hardware send us the current patch state
    midiToAnyma->sendMessageNow( MidiMessage( patchSyx, 7, 0 ) );
    
    // begin anyma editor mode and request regular updates
    Thread::sleep( 1000 ); // kludgey delay to allow patch dump to complete
    midiToAnyma->sendMessageNow( MidiMessage( eModeSyx, 7, 0 ) );
    anymaKeepAlive.start();
    anymaGetStatus.start();
  }
  
  void stop()
  {
    // cease transmision of editor status requests
    anymaKeepAlive.stop();
    anymaGetStatus.stop();
    
    // request the anyma hardware send us the current patch state
    // kludgey delay to allow state to change
    Thread::sleep( 5000 );
    midiToAnyma->sendMessageNow( MidiMessage( patchSyx, 7, 0 ) );
  }
  
  bool isActive(){ return anymaKeepAlive.isActive(); }
  void toggle(){ isActive() ? stop() : start(); }

  void handleIncomingMidiMessage (juce::MidiInput* source, const juce::MidiMessage& message) override
  {
      // DBG( "incomingMIDI " + String( message.getRawDataSize() ) );
    
      if ( message.getSysExDataSize() >= 256 ) // is sysex patchdump?
      {
        midiToSequencer->sendMessageNow( message ); // forward to sequencer
      }

      if( message.isSysEx() && message.getSysExDataSize() < 256 ) // is sysex param state?
      {
        const uint8_t* rx = message.getRawData(); // incl 0xF0 and 0xF7 terminator
        handleMainMatrix( rx );
        handleAltMatrix( rx );
      }
    
  }

  void setInputFromAnyma( const int index )
  {
    auto list = juce::MidiInput::getDevices();
    deviceManager.removeMidiInputCallback(list[fromAnymaIndex], this);

    auto newInput = list[index];
    if (! deviceManager.isMidiInputEnabled (newInput) )
        deviceManager.setMidiInputEnabled (newInput, true);
    deviceManager.addMidiInputCallback (newInput, this);
    
    fromAnymaIndex = index;
  }

  void setOutputToAnyma( const int index )
  {
      midiToAnyma = juce::MidiOutput::openDevice( index );
      anymaGetStatus.setOutput( midiToAnyma );
      anymaKeepAlive.setOutput( midiToAnyma );
  }
  
  void setOutputToSequencer( String midiToSequencerDeviceName )
  {
    midiToSequencer = juce::MidiOutput::createNewDevice(midiToSequencerDeviceName);
    if( midiToSequencer == nullptr )
    {
         std::cout << "ERROR creating virtual midi port\n";
         return;
    }
  }

  void handleAltMatrix( const uint8_t* rx )
  {
    if( rx == nullptr ) return;
    if( midiToSequencer == nullptr ) return;
    
    // set ccNum and ccVal from rx data, -1 if not found
    int16_t ccNum = -1;
    int16_t ccVal = -1;
  }

  void handleMainMatrix( const uint8_t* rx )
  {
    if( rx == nullptr ) return;
    if( midiToSequencer == nullptr ) return;
    
    // set ccNum and ccVal from rx data, -1 if not found
    int16_t ccNum = -1;
    int16_t ccVal = -1;

    if( 0xF0 == rx[0] && // is 'tuning' param?
        0x71 == rx[1] &&
        0x00 == rx[2] &&
        0x02 == rx[3] )
    {
        ccNum = 23;
        ccVal = rx[4];
    }

    if( 0xF0 == rx[0] && // is matrix param?
        0x71 == rx[1] &&
        0x06 == rx[2] )
    {
      switch( rx[3] )
      {
        case 0: ccNum = 16; ccVal = rx[4]; break;
        case 1: ccNum = 17; ccVal = rx[4]; break;
        case 2: ccNum = 18; ccVal = rx[4]; break;
        case 3: ccNum = 19; ccVal = rx[4]; break;
        case 4: ccNum = 20; ccVal = rx[4]; break;
        case 5: ccNum = 21; ccVal = rx[4]; break;
        case 6: ccNum = 22; ccVal = rx[4]; break;
        
        // cc 23 'tuning' handled elsewhere
        
        case 7: ccNum = 24; ccVal = rx[4]; break;
        case 8: ccNum = 25; ccVal = rx[4]; break;
        case 9: ccNum = 26; ccVal = rx[4]; break;
        case 10: ccNum = 27; ccVal = rx[4]; break;
        case 11: ccNum = 28; ccVal = rx[4]; break;
        case 12: ccNum = 29; ccVal = rx[4]; break;
        case 13: ccNum = 30; ccVal = rx[4]; break;
        case 14: ccNum = 31; ccVal = rx[4]; break;
      }
      
      if( -1 != ccNum && -1 != ccVal ) // is valid?
      {
        MidiMessage tx = MidiMessage::controllerEvent( 1, ccNum, ccVal );
        midiToSequencer->sendMessageNow( tx );
      } // is valid?
    }// is matrix param?
  }
};

class MidiProcessorComponent
  : public juce::Component
  , private juce::ComboBox::Listener
{
private:
  juce::Label& uiLabel_mainStatus; // parent ref
  MidiProcessor procr; // logic and MIDI tx/rx

  juce::Colour uiColour_backGrey = juce::Colour(0xff333333);
  juce::Colour uiColour_transpGrey = juce::Colour(0x77000000);
  
  juce::Colour uiColour_background = uiColour_transpGrey;

  juce::ComboBox uiCombo_midiFromAnyma; // choose midi in port
  juce::Label uiLabel_midiFromAnyma;
  
  juce::ComboBox uiCombo_midiToAnyma; // choose midi out port
  juce::Label uiLabel_midiToAnyma;
  juce::Label uiLabel_midiToSequencer; // show virtual port name
  
  juce::Label uiLabel_info; // "the problem, this solution"
  
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MidiProcessorComponent);
  
public:
  MidiProcessorComponent( juce::Label& mainStatus )
    : uiLabel_mainStatus( mainStatus )
  {
    // //// ////  //// ////  //// ////  //// ////  //// ////  //// ////
    // the problem - this solution
    addAndMakeVisible( uiLabel_info );
    Font uiInfoFont = Font( Font::getDefaultMonospacedFontName(), 10, 0 );
    
    uiLabel_info.setFont( uiInfoFont );
    uiLabel_info.setJustificationType( Justification::topLeft );
    uiLabel_info.setText(
      "\n" \
      "The Problem:\n" \
      "-Anyma wants MIDI CC.\n" \
      "-Sequencer edits CC.\n" \
      "-Anyma sends SYSEX.\n" \
      "\n" \
      "Anyma patch state changes may not be" \
      " simply recorded/edited with a MIDI sequencer.\n" \
      "\n" \
      "This solution:\n" \
      "-Pal maps SYSEX > CC.\n" \
      "-You view/edit CC.\n" \
      "-Anyma receives CC.\n" \
      "\n" \
      "AnymaPal also requests and relays your patch" \
      " state as SYSEX (so you can capture the entire" \
      " Anyma state in your sequencer.) Happy! :)",
      dontSendNotification
    );
  
    // //// ////  //// ////  //// ////  //// ////  //// ////  //// ////
    // user interface dropdown menu
    addAndMakeVisible (uiLabel_midiFromAnyma);
    uiLabel_midiFromAnyma.setText ("From Anyma:", juce::dontSendNotification);
    
    addAndMakeVisible (uiCombo_midiFromAnyma);
    uiApplyComboColours (uiCombo_midiFromAnyma, ComboType::InputFromAnyma);
    uiCombo_midiFromAnyma.addListener( this ); // comboBoxChanged
    uiCombo_midiFromAnyma.setTextWhenNoChoicesAvailable ("Unvailable");
    
    // calls chooseMidiInput(), uiCombo_midiInput.setSelectedId etc
    uiRefreshMidiInputList(-1, "Anyma Phi"); // -1 means "no preferred dev index"
    
    // //// ////  //// ////  //// ////  //// ////  //// ////  //// ////
    // user interface dropdown menu
    addAndMakeVisible (uiLabel_midiToAnyma);
    uiLabel_midiToAnyma.setText ("To Anyma:", juce::dontSendNotification);

    addAndMakeVisible (uiCombo_midiToAnyma);
    uiApplyComboColours (uiCombo_midiToAnyma, ComboType::OutputToAnyma);
    uiCombo_midiToAnyma.addListener( this ); // comboBoxChanged
    uiCombo_midiToAnyma.setTextWhenNoChoicesAvailable ("Unvailable");
    
    // calls chooseMidiInput(), uiCombo_midiInput.setSelectedId etc
    uiRefreshMidiOutputList(-1, "Anyma Phi"); // -1 means "no preferred dev index"
    
    // //// ////  //// ////  //// ////  //// ////  //// ////  //// ////
    // virtual (macos) coremidi port
    String midiToSequencerDeviceName = "from Anyma Pal";
    procr.setOutputToSequencer( midiToSequencerDeviceName );
    
    // //// ////  //// ////  //// ////  //// ////  //// ////  //// ////
    // user interface label
    addAndMakeVisible (uiLabel_midiToSequencer);
    uiLabel_midiToSequencer.setText ("To Sequencer: " + midiToSequencerDeviceName, juce::dontSendNotification);
  }
  
  ~MidiProcessorComponent()
  {
  }
  
  //====================================================================
  // passthru to data processor
  void toggle(){ procr.toggle(); }
  bool isActive(){ return procr.isActive(); }

  //====================================================================
#pragma mark ui event callbacks

  void comboBoxChanged( ComboBox* comboBoxThatHasChanged ) override
  {
    auto newIndex = comboBoxThatHasChanged->getSelectedItemIndex();
  
    if( comboBoxThatHasChanged == &uiCombo_midiFromAnyma ) chooseMidiInput ( newIndex );
    if( comboBoxThatHasChanged == &uiCombo_midiToAnyma ) chooseMidiOutput ( newIndex );
  }

  //==================================================================

  void chooseMidiInput( int index )
  {
      procr.setInputFromAnyma(index);
      uiCombo_midiFromAnyma.setSelectedId (index + 1, juce::dontSendNotification);
  }

  void chooseMidiOutput( int index )
  {
      procr.setOutputToAnyma( index );
      uiCombo_midiToAnyma.setSelectedId( index + 1, juce::dontSendNotification );
  }
  
  //================================================================
#pragma mark ui related

  enum ComboType {
    InputFromAnyma = 0,
    OutputToAnyma,
    OutputToSequencer
  };

  void uiApplyComboColours(juce::ComboBox& uiCombo, ComboType type)
  {
    juce::Colour mainColour = uiColour_backGrey;
    juce::Colour accentColour = Colours::grey;
    
    if( type == ComboType::InputFromAnyma ) accentColour = juce::Colours::yellow;
    if( type == ComboType::OutputToAnyma ) accentColour = juce::Colours::orange;
    if( type == ComboType::OutputToSequencer ) accentColour = juce::Colours::lightcyan;
  
    uiCombo.setColour(juce::ComboBox::ColourIds::backgroundColourId, mainColour);
    uiCombo.setColour(juce::ComboBox::ColourIds::arrowColourId, mainColour);
    uiCombo.setColour(juce::ComboBox::ColourIds::buttonColourId, accentColour);
    uiCombo.setColour(juce::ComboBox::ColourIds::textColourId, accentColour);
    
    // BUG: JUCE v3 cant config popupmenu colours - still broken in JUCE v7 in 2023
    uiCombo.setColour(juce::PopupMenu::ColourIds::backgroundColourId, mainColour );
    uiCombo.setColour(juce::PopupMenu::ColourIds::textColourId, accentColour );
  }
  
  void uiApplyLabelColours(juce::Label& uiLabel)
  {
    uiLabel.setColour(juce::Label::ColourIds::backgroundColourId, uiColour_backGrey);
    uiLabel.setColour(juce::Label::ColourIds::textColourId, Colours::grey);
  }
  
  void uiApplyTextButtonColours(juce::TextButton& uiTextButton)
  {
    uiTextButton.setColour(juce::TextButton::ColourIds::buttonColourId, uiColour_backGrey);
    uiTextButton.setColour(juce::TextButton::ColourIds::textColourOffId, Colours::darkred);
  }
  
  /** input device names */
  juce::StringArray uiRefreshMidiInputList( int index, juce::String preferredName )
  {
      juce::StringArray midiInputs = juce::MidiInput::getDevices();
    
      juce::StringArray midiInputNames;
      for (auto input : midiInputs)
          midiInputNames.add (input); // .add(input.name); ?
      uiCombo_midiFromAnyma.clear();
      uiCombo_midiFromAnyma.addItemList (midiInputNames, 1);
  
      if( index == -1 ) index = midiInputs.indexOf( preferredName );
      if( index == -1 ) index = 0;
  
      chooseMidiInput( index );
    
      return midiInputs;
  }
  
  /** output device names */
  juce::StringArray uiRefreshMidiOutputList( int index, juce::String preferredName )
  {
      juce::StringArray midiOutputs = juce::MidiOutput::getDevices();
      juce::StringArray midiOutputNames;
      for (auto input : midiOutputs)
          midiOutputNames.add (input); // .add(input.name); ?
      uiCombo_midiToAnyma.clear();
      uiCombo_midiToAnyma.addItemList (midiOutputNames, 1);
    
      if( index == -1 ) index = midiOutputs.indexOf( preferredName );
      if( index == -1 ) index = 0;
  
      chooseMidiOutput( index );
    
      return midiOutputs;
  }
  
  //=======================================================================
#pragma ui component level

  void paint (Graphics& g) override
  {
      g.fillAll( uiColour_background );
  }

  void resized() override
  {
      int padHeight = 10;
      auto initialarea = getLocalBounds();
    
      // horizontal two thirds
      auto area = initialarea.withWidth( 2 * initialarea.getWidth() / 3 );
      uiLabel_midiFromAnyma.setBounds( area.removeFromTop(36).reduced(4) );
      uiCombo_midiFromAnyma.setBounds( area.removeFromTop( uiLabel_midiFromAnyma.getHeight() ) );

      area = initialarea.withWidth( 2 * initialarea.getWidth() / 3 );
      area.setTop( uiCombo_midiFromAnyma.getBottom() + padHeight );
    
      uiLabel_midiToAnyma.setBounds( area.removeFromTop(36).reduced(4) );
      uiCombo_midiToAnyma.setBounds( area.removeFromTop( uiLabel_midiToAnyma.getHeight() ) );
    
      area.setTop( uiCombo_midiToAnyma.getBottom() + padHeight );
      uiLabel_midiToSequencer.setBounds( area.removeFromTop(36).reduced(4) );
    
      // horizontal one third
      area = initialarea
             .withLeft( 2 * initialarea.getWidth() / 3 )
             .withWidth( initialarea.getWidth() / 3 );
      uiLabel_info.setBounds( area.withTrimmedBottom( 36 ) );
  }
};
