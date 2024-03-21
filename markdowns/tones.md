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

Sine actually makes use of its own accumulators called `phase_counter`s, which count incrementally because these values act as indices: each counter is modified via some calculations and used to index the large, pre-computed sine-value table, which is reduced to the desired range and the `Vout` value is then accumulated by `playNotes`, just as with the other tones.

## Volume

One interesting note is the perceived volumes of each of the waveforms: sawtooth and square seemed louder than triangle or sine at the same volume. I think this is because our easier are sensitive to frequency and higher frequencies at the same volume tend to sound a bit louder. Because sawtooth and square have abrupt waveform changes, i.e. very high frequency changes, they sound a lot louder to us; the triangle waves changes are not as drastic, so it sounds a little quieter, and because the sine wave is very smooth, it sounds the quietest to us, despite being at the same volume as the others.
