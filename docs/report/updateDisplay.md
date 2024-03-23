# Task: updateDisplay

The updateDisplay task takes global variables with mutex and stores them in global variables, which display current information onto the OLED display.

# Task type and functions

The task is done as a thread as in order to update the display at a fixed interval so that the updated values from the updateKey task is finished so that the correct information is displayed. Making it a thread would create a high overhead without much advantage, as the change in the refresh rate is negligibly noticeable to the user. 

The only function that is used in this task is the u8g2 library, which displays the localised variable onto the display.

## Task interval and execution time

The initiation interval is set to 100ms.

The characterisation of updateDisplayTask followed a structured approach, similar to that of the updateKeys task, to accurately measure its execution time:

- Similar to the characterisation of other tasks, we disabled all unrelated tasks and ISRs to isolate updateDisplayTask and minimise external influences on its performance.
- The task was configured to run outside its normal while(1) loop for a single iteration/
- No additional modifications were deemed necessary to simulate a worst-case execution scenario.
- We measured the task's execution time over 32 iterations.

Assumptions:

- The system was assumed to be in a controlled state with no external interactions affecting the task's execution.
- The task's interaction with the u8g2 driver and the display hardware was assumed to represent the primary workload, with minimal overhead from other operations.

Results:

| Initiation Interval (ms) | Execution Time (ms) |
| --- | --- |
| 100.00 | 16.778 |
- The initiation interval is set timed execution within the threaded system of this task.
- Updating a display requires processing graphical data for each pixel, which is inherently more time-consuming than tasks with simpler computational requirements and this is reflected in the high execution time.

## Data/resource use and synchronisation

The task uses a plethora of system variables listed below:

- sysState.keyStrings - Information of currently pressed keys
- sysState.octave - Information of currently selected octave
- sysState.TX_Message[0] - Information of whether is message is being sent over CAN
- sysState.met - Information of currently selected metronome BPM
- sysState.metOnState - Information of whether the metMenu is selected or not
- sysState.menuState - Information of which selectable menu item is selected, is can be
    - 0 -Metronome Menu
    - 1 - Recording
    - 2 - Octave Selection
- sysState.isSelected - information of whether the current menuState is selected or not
- sysState.knobValues - Information on the knob values to show information of
    - knobValues[0] - Volume (1-8)
    - knobValues[1] - Tone (Saw, Square, Sine, Triangle)
    - knobValues[2] - Audio setting (Vibrato)
    - knobValues[3] - Echo setting (Reverse)
- sysState.dotLocation - coordinates of the dot location that shows the user which menu is currently selected

The synchronisation is done through the use of mutex commands, which will block other calls to the variable to a waiting state until the block is removed. For the purpose of organisation and to avoid having multiple calls in the waiting state, the sysState variables are all called at the start of the task.

## Blocking dependencies and deadlock risk

The only blocking action that happens in this task is at the start when all the variables are taken. However, the mutex is then returned meaning that deadlock is not likely to happen. As there are also only one dependency, and no cycle of dependencies, a deadlock will not occur.
