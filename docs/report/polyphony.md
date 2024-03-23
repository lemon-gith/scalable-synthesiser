# Polyphony

This feature allows more than one note to be played, audibly, at a time (since, before, multiple notes would result in screeching…), as well as now being able to play chords.
This feature required a change in the way that notes were handled, since one, general accumulator would just accumulate the steps of every note into it, which was causing the frequency-addition (high-pitch), and adding the Vouts from all the notes (which was essentially n + (n + 1) + …) caused amplitude accumulation to occur, resulting in the high-pitch, loud screech.

## Implementation & Usage

The key breakthrough for implementing polyphony was recognising, that in order to have each note independently contribute its frequency, each note needed its own accumulator. 
This meant then expanding the single accumulator into an array of accumulators, which, in order to account for all notes in all octaves (played by any connected device), is very wide.

This was the primary change, with most of the other modifications being compatability modifications or fixes to resultant bugs. 

The previous `playNote` function was ‘upgraded’ into the `playNotes` function that makes use of a new `keys_down` global array that keeps track of which keys are currently being pressed (across all keyboards). For depressed keys, `playFunction` is called on them (yes, even for sampled waveforms: bad practice, perhaps :}), which then produces the voltage corresponding to that tone and volume, which is accumulated across all key-presses to produce the output amplitude. 

This accumulation can result in a series of amplitudes that are too large to be reproduced by the speaker, which means that these bits of the waveforms must be clipped, which leads to undesirable crackling sounds (high frequency pops) from the abrupt waveform change caused by clipping.

The `phase_accumulators` array is kept within the function as a static variable, since it’s not used by any other function, which affords the performance advantage of not needing to access a semaphore. This is unlike the `keys_down` array, which is accessed by many other functions for both writing and reading purposes.

The core usage of the wider accumulators is when `playFunction` is used to add the modified `stepSize` (`phase_inc`) to the accumulator:


``phase_accumulators[idx] += phase_inc;``

## Notes

These wider accumulators actually caused problems to flare up all throughout the codebase, because a lot of things had only been designed to work with a single accumulator variable and to only really handle the last key pressed, so a lot of the polyphony work was just due to the need to go through and update a lot of the existing code to be compatible with this, rather major, change.
