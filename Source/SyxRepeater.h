/*
  SyxRepeater
  Send (repeatedly, at timed intervals) midi system exclusive msgs.
  Used to TX keepalive and reportstatus commands to anyma hardware
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

//
class SyxRepeater : private juce::Timer
{
private:
  MidiOutput* midiOutput;
  MidiMessage msg; // default is empty sysex message
  unsigned int interval = 1000;

public:
  void setMsg( const uint8_t* data, const unsigned int numBytes )
  { msg = MidiMessage( data, numBytes, 0); }
  
  void setOutput( MidiOutput* outputPort )
  { midiOutput = outputPort; }
  
  void setInterval( const unsigned int newInterval )
  { interval = (newInterval) ? newInterval : 1; }

  bool isActive(){ return isTimerRunning(); }
  void stop() { stopTimer(); }
  void start()
  {
    if( isActive() ) return; // abort if already running
    else startTimer( interval );
  }
  void toggle(){ isActive() ? stop() : start(); }
  
  void timerCallback()
  {
    if( msg.getSysExDataSize() == 0 ) return;
    if( midiOutput == nullptr ) return;
    midiOutput->sendMessageNow(msg);
  }
};
