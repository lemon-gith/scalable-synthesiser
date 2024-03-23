# Octaves

This feature allows the keys to play more than the note 12 frequencies. It allows the user to shift the octaves up or down, which doubles/halves the frequencies output by keypresses.

## Implementation & Usage

Keeping track of which octave a key is in is done per synth, since each synth can only be in one octave at a time. This is therefore stored in the `sysState` struct as a volatile variable (since it needs to be read by many different tasks).

The variable is only written to via the UI, controlled by a menu item dedicated to changing octaves via the joystick. The `navigate` function creates a local copy of the current octave (between `xSemaphoreGive/Take`), and modifies it according to the current menu (and joystick) activity, before then writing it back to `sysState`.

The primary usage of octaves is in the note-playing functions, where it’s used to identify how to scale the frequencies:

```cpp
  uint32_t phase_inc = (oct < 4) ? 
    (stepSizes[note] >> (4 - oct)) : 
    (stepSizes[note] << (oct - 4));
```

In mathematical terms, moving up and down octaves consists of doubling or halving the frequencies of the corresponding notes. In order to implement this, the constant array of the 12 step sizes is bitshifted (multiply by +/- power of 2, which is a very efficient operation for the CPU) to apply the relevant scalings to the stepSize increment sizes. This scales the speed at which the amplitude increases and wraps around, which scales the frequency.

## Notes

After implementing polyphony, octaves have become a computed attribute of the key index (used in `keys_down` and other similar arrays), along with `note`. However, local-note computation is quicker and code is more readable when the value is not continually being calculated, so whether it is computed once or copied from `sysState`, it is still preferable and more thread safe to have a local `octave` value for use.

One downside of storing the variable in `sysState` is that taking the mutex to copy the value of `octave` now blocks other operations. However, it is important for the UI and for local key presses to be able to read which octave the synth is currently on, which necessitates the global storage.