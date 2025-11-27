# anymaPal
A friend for MIDI recording with Aodyo Anyma Phi

![robot by catalyststuff](Source/anymaPal.png)

## The problem
It's not possible to "just jam" with Anyma Phi whilst a MIDI sequencer captures your patch edits and state changes. Anyma patch state changes may not be simply recorded/edited.

- Anyma wants MIDI CC.
- Your MIDI sequencer edits CC.
- Anyma sends SYSEX.

## This solution
Simply record output from Pal and your controller during a "take" where you are adjusting Anyma parameters.

- Pal converts SYSEX to CC.
- Anyma receives CC.
- Your MIDI sequencer edits CC.
- Everyone is happy.
  
## Extra info
AnymaPal also requests and relays your patch state as SYSEX (so you can capture the entire Anyma state in your sequencer before each take).

Happy recording! :)

For a DAWless alternative solution, see [anymaHWPal a hardware friend](//github.com/uwePhillPhelps/anymaHWPal/).
