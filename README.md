# anymaPal
a friend for MIDI recording with Aodyo Anyma Phi

![robot by catalyststuff](Source/anymaPal.png)

## The problem
Anyma patch state changes may not be simply recorded/edited with a MIDI sequencer.

- Anyma wants MIDI CC.
- Your MIDI sequencer edits CC.
- Anyma sends SYSEX.

## This solution
Record output from Pal and your controller during a "take" where you are adjusting Anyma parameters.

- Pal converts SYSEX to CC.
- Anyma receives CC.

## Extra info
AnymaPal also requests and relays your patch state as SYSEX (so you can capture the entire Anyma state in your sequencer).

Happy recording! :)
