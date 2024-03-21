# UI

This feature provides a user-friendly experience to display useful information for users and to allow them to easily toggle device configurations. The display primarily shows a few frames displaying information, such as the notes currently being played, the current volume, tone, octave, etc., to providing a menu for users to be able to change modes and modify features like the octave, metronome (bpm and switch), and playback.

## Usage

While the data displayed by the UI spans most of the codebase, the primary function implementing the UI is the `updateDisplayTask` task. 

This task sets the display to update at 10Hz, using `vTaskDelayUntil` to keep track of when it should run, and after clearing the write buffer (clean slate to write on), it takes the `sysState` mutex, in order to copy a *lot* of relevant values to local variables.

As mentioned above, the `drawFrame` function is used to put frames around the elements we want framed, then the `drawCursor` function is used to put a little cursor on the currently selected menu item. If the cursor is `-` it’s just hovering, but if the joystick button is pressed it toggles the cursor between hovering and `>` (active). The cursor moves according to the shape of the menu, not the shape of the array, so moving it down from Metronome, would bring it down to Playback, and left from there would bring it to Octave, and vice versa—invalid movements are ignored for usability.

The cursor is the primary form of interaction, purely between the user and the menu, everything else primarily affects a feature of the synth, such as the volume selection modifying how much the output amplitude is attenuated, or the metronome’s bpm affecting the frequency of the metronome’s blips. These configurations are selected via the menu.

## Implementation

Now for the details that no-one notices unless they’re not there:

1. A button-press state machine has been created for the joystick button presses, so that they’re essentially edge-triggered
2. A state-like arming implementation, has also been created for the octave traversing, to shift the octave by 1 per ‘joystick flick’
3. Setting certain regions of the joystick so that they correspond to up, down, left, and right

1. In `state_machines.h`, a `ButtonPress` struct that acts as a state machine has been created to handle all the arming logic of the button press: should the joystick be depressed from rest, it will trigger the button to become armed for one ‘cycle’, and should it remain depressed on the next ‘cycle’, it now switches to an `ON` state, instead of endlessly toggling the selection between active and hovering. The `OFF` state is just a convenient way of saying that the button is no longer depressed, but the `ON` and `OFF` states perform roughly the same job.

2. The old implementation made it very difficult to change octaves, since it would just rush through them as long as the joystick wasn’t at rest, so this SM-like method was developed to only execute when a change in joystick direction is detected. It’s very simple and just keeps track of the previous direction in a static variable, doing nothing if the incoming value is the same, and executing an action (such as octave shifting) only if they differ.

It was implemented like this, because the extra overhead of a state machine is more cumbersome in this case, since its use cases will be a lot more diverse and case-specific, this way, each element that needs to use joystick values can execute its own relevant logic: 

- `octave` needs to keep hold of the previous direction, to only execute on changes
- metronome’s bpm doesn’t store anything, since it should move quickly through it’s values
- menu-item selection should *also* be handled by its own logic, due to its different use-case

3. The joystick’s direction was read by taking the `analogRead` value of the joystick’s x and y axis, which were then put through different if conditions to yield a direction. It checked if these values were over or under certain boundaries which would create regions, defining different directions, to increase the vaild region available to the user. 
Moreover, because a user may not push the joystick perfectly to the ‘up’ direction, for example, these regions afford the user more robustness in how much of the joystick zone accounts for each direction region. On the other hand, there is also a ‘dead-zone’ or ‘base’ zone on the joystick, to which there is no direction associated, allowing the system to ignore slight bumps to the joystick.

TODO: Link & Embed images/videos when adding it to the github, using `![]()` syntax