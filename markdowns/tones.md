# Tones

This feature allows the timbre of the output signal to be changed by the user.

## Implementation & Usage

### Sawtooth

From the labs, this uses a phase accumulator to accumulate an overflow-able count for each note, that increases linearly, before overflowing to become negative again (dropping back to its start).

### Square

This makes use of the phase accumulator, by thresholding it to either `127` or `-128`, depending on whether or not the `phase_out` value was above or below 0.

### Triangle

This, too, makes use of the phase accumulator, by reflecting it in $y=0, \forall y > 0$, i.e. all positive values were multiplied by $-1$, before  

### Sine

Sine actually makes use of its own accumulators called `phase_counter`s, which count incrementally because these values act as indices: each counter is used to index


## Notes

After implementing polyphony, octaves have become somewhat of a computed attribute of the key index (used in `keys_down` and other similar arrays), along with `note`. But, it makes local-note computation quicker and the code more readable to not have to keep calculating it, so whether it’s computed once or taken from `sysState`, it’s still preferable to have a local `octave` value for use.

One downside of keeping it in `sysState` is that taking the mutex to copy the value of `octave` now blocks other operations, but it’s still important for the UI and for local key presses to know which octave the synth is currently on, so it’s still necessary to keep around in the global struct.
