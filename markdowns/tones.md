# Tones

This feature allows the 

## Implementation & Usage

### Sawtooth

oof

### Square

hmm

### Triangle

aaaah

### Sine

hehe

Keeping track of which octave a key is in is done per synth, since each synth can only be in one octave at a time… and thus it’s stored in the `sysState` struct (as a volatile variable, since it’s read by many different tasks).

It’s only written to via the UI, where there’s a menu item dedicated to changing octaves via the joystick. The `navigate` function grabs a local copy of the current octave (between `xSemaphoreGive/Take`), and modifies it according to the current menu (and joystick) activity, before then writing it back to `sysState`.

The primary usage of octaves is in the note-playing functions, where it’s used to identify how to scale the frequencies:

``uint32_t phaseInc = (oct < 4) ? 
    (stepSizes[note] >> (4 - oct)) : 
    (stepSizes[note] << (oct - 4));``

In mathematical terms, going up and down octaves just consists of doubling or halving the frequencies of the corresponding notes, so in order to implement that, we take our 12-array of step sizes and use bitshifting (multiply by +/- power of 2) to apply the relevant scalings to the stepSize increment sizes, which scales the speed at which the amplitude increases and wraps around, which scales our frequency :)

## Notes

After implementing polyphony, octaves have become somewhat of a computed attribute of the key index (used in `keys_down` and other similar arrays), along with `note`. But, it makes local-note computation quicker and the code more readable to not have to keep calculating it, so whether it’s computed once or taken from `sysState`, it’s still preferable to have a local `octave` value for use.

One downside of keeping it in `sysState` is that taking the mutex to copy the value of `octave` now blocks other operations, but it’s still important for the UI and for local key presses to know which octave the synth is currently on, so it’s still necessary to keep around in the global struct.
