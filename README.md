# Embedded Systems: Synthesiser

## Intro

A second year group project for a module on Embedded Systems.\
The goal was to write all the practical code for a synthesiser that stacks with other synths, 
including for keypress detection, noise generation, communication, etc.

The synthesiser uses an `ST NUCLEO-L432KC` board, running an `STM32L432KCU6U` processor, with an `ARM Cortex-M4` core.
<div style="text-align:center;">
    <img src="./docs/getting_started/images/synth.png" style="width: 450px;">
</div>

(details courtesy of @edstott)

For more details on the board itself and the basic features that we implemented, the [getting started](docs/getting_started/) section of the `docs`, also courtesy of @edstott, does a good job of explaining everything.

## What we accomplished

Over the span of the coursework, we completed all the basic features (such as basic noise generation and simple communications), so we started implementing more and more _advanced_ features, primarily: 
- An [actual UI](docs/report/UI.md)
- [Multiple Octaves](docs/report/octaves.md)
- [Alternate Timbres](docs/report/tones.md) (primarily Sine)
- [Polyphony](docs/report/polyphony.md) (playing more than one note at a time)
- A [Metronome](docs/report/Metronome.md)
- Among other features

More details regarding any of the implemented features can be found in our coursework [report](docs/report/).

## Evaluation

### UI/UX

The UI/UX was designed with ease-of-use in mind, so the (easier-to-use) knobs are designed to change the settings that a user would likely change more frequently.\
The display tries to show all the relevant information regarding the current state of the synth, from volume, octave, and active menu selection, to current keypresses and receiver/listener configuration. 

It does a good job of meeting keeping the user informed of the state, displaying all the information in highlighted frames and using visual changes via the cursor, to tell the user where and in what state the current menu selection is. The frames are also well labelled, with the key-press frame being self-explanatory when you press keys.

One glaring issue is the font: the display is too small to easily display all the information needed, so the sacrifice made was to pick a very small font, that isn't the most legible, but can still be read.\
Fixing this would require a more legible font that maintains this size, or reducing the amount of information displayed on the screen at one time.

The input side of things is also well taken care of, the knob-reading is fairly accurate and the button presses are read well. The joystick, the fiddliest input, has been designed in a bug-resilient way, with direction regions being defined to allow for leeway on the user's side.\
The wraparound used for menu items makes for very convenient selections for octaves and metronome bpm halves the maximum amount of time a user needs to spend going through the range of values. The state-machine-like behaviour implemented for the joystick's button press and the octave shifting make them much less fiddly, since they lock into a selection for a single button press, instead of continuously toggling while the button is pressed.

### Noise Generation (& Volume)

The goal was to use simple calculations to generate the amplitudes for all the required tones, at a `22kHz` sample rate. 

The sawtooth and triangle waves worked without hitches, using the same basic phase-accumulator mechanism. The square wave also intelligently makes use of the phase-accumulator by thresholding its value. The sine wave chooses to use a separate counter for its own calculations, which does incur an increase in calculation complexity, but generates a clean sine wave for it. 

The volumes of each of the different tones poses an interesting problem: since human hearing is more sensitive to higher frequencies, the sharper waveforms seem louder, despite having the same amplitudes. However, this results in a disconnect between the volumes of each individual waveform at one volume setting.

Currently, this is not fully addressed, and the sine wave is noticeably quieter than the other waveforms, with the triangle being quieter than both the sawtooth and square.\
This could be solved by either boosting the amplitude of the smoother waveforms or by attenuating the amplitude of the sharper waveforms (the latter would probably be safer for the speaker).

### Octaves & Polyphony

The ability to shift the pitches of the current keyboard up and down in octaves, as well as the ability to play (and hear) multiple different tones at once. These are major UX improvement features, if one can call them that.

However, as they are both linked to noise generation, the major issues that implementing Octaves and Polyphony cause are issues relating to maxing-out the amplitude of the output sound, forcing clipping of the waveforms and producing noisy, broad-frequency crackling noises.

This may require a bit of planning and re-thinking of the noise generation pipeline to solve. Something that takes into account the final output amplitude and ensures that it doesn't exceed `analogWrite`'s limit of 255, and something that minimises the amount of infrastructure needed to produce sounds, e.g. reducing large LUTs, reducing number of different counters, etc.

### Metronome

The ability to keep a constant beat for the user by producing a 'blip' at regular intervals. This currently works very well and melds with the noise generation very smoothly, since it doesn't interrupt general function, only inserting a little blip when it needs to. Has no major flaws and works well, in my opinion.

### Communications

This enables multiple synths to pass messages to each other, establishing a master synth and allowing the sound of all the accumulated playing notes to be generated and output by one keyboard.

The current implementation of communication is probably the weakest feature on the synth. It's very minimal, only allowing a single external keypress to be registered every 'noise-gen cycle', with no method of communicating that a key is no longer being pressed. This leads to a fun bug, whereby if you hold down an external key and switch octaves, that tone is held, because the code only refreshes the current octaves being played.

A fix would likely require rewriting the communication protocol to send more information per message between synths, and rewriting the message-handling to handle any and all incoming messages received, including key-unpresses.

### Playback

This feature would enable the user to start recording, play a passage, stop recording, then later play this passage back, with the exact same sound production as before.\
This feature was only experimentally implemented and is not fully functioning at the time of writing. 

### Codebase

The codebase currently consists of one primary [`main` file](./src/main.cpp), that contains all of the functional code, with a couple of header files that contains constant definitions and type/struct definitions. At the very least, the file has been sectioned out into its functional components, each section performing one primary duty, e.g. Noise-Gen, Reading Inputs, etc.

This is, however, a bit bulky and not well-separated: all the code is in one massive file. It could be improved by splitting up the primary sections into their own respective files, using header files to carry the shared information across files.\
When this was attempted earlier, errors with file inclusions made this rewriting infeasible under time restrictions.

## Improvements

### Codebase

<!-- TODO: write sth after implementing -->

### Communications

<!-- TODO: write sth after implementing -->

### Noise Generation (Volume)

<!-- TODO: write sth after implementing -->

### UI/UX

<!-- TODO: write sth after implementing -->

### Envelope

Actually, another _advanced_ feature: this is one we didn't have time to implement during the project. 

<!-- TODO: write sth after implementing -->

### Playback

<!-- TODO: write sth after implementing -->

<!-- -------------------------- END OF SECTION -------------------------- -->

<!-- Sth. about how this, of course, isn't perfect and whatnot, but that this is now a project that works and I'm happy with? -->

## TODOs (-> Improvements)

Things that I wanted to improve on after the coursework deadline:

0. Write more in this README :p
1. Break this code into a properly structured project
2. Implement functional and scalable communications
   - current communication protocol is lackluster and buggy, to say the least
   - 8-bit, on/off stuff, implement main/sub, dominance establishment protocol
3. Fix tone volumes, due to the nature of the waveforms (and maybe gen.), some sound a lot quieter
4. Improve UI, remove bugs, improve readability, make note display uniform
5. Implement sound enveloping (toggled by volume knob)
6. Integrate Playback feature (MITeo21's branch)
7. More stuff?
