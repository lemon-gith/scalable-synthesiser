# Task: sampleISR

This task is the interrupt service routine which outputs sound from the speaker at a specified volume, tone and sound.

## Task type and functions

This task is an interrupt as it requires a high frequency of 22 kHz in order to produce a smooth, coherent sound for human ears, and is also critical to the success of the system, since it will be audible if the tasks deadline is not met every time. Additionally, due to its frequent use, making it a task would create delays in the deadlines for the freeRTOS scheduler, and will be too complicated to analyse timing intervals.

The main function called by this task is the playNotes function which calculates and returns the voltage to be sent to the speaker. Within the playNotes function, two more functions are used, playFunction and playMetronome. playFunction is used so that it can transform the given note to output in the selected waveform i.e. square, sine, triangle or sawtooth. As the sine wave is a sampled wave rather than transformed, it is calculated in the playSampled function. Similarly, the playMetronome function produces a metronome beat based on the BPM set by the user (via the UI). 

## Task interval and execution time

The frequency for this interrupt is constant at 22kHz. This frequency is set so that the digital waves output by the speaker have a defined sample frequency. If this interrupt triggers 22,000 times a second, its initiation interval is 1 second/22,000 = 45.4545…μs. 

- Calculating the execution time
    - Explain measurement method
 
| Initiation Interval (ms) | Execution Time (ms) |
| --- | --- |
| - | 0.0455 |

## Data/resource use and synchronisation

The task uses multiple sysState variables for to calculate the required output voltage for the speaker. As the task is an interrupt instead of a thread, it cannot be mutexed, and hence is atomically loaded instead. The list of sysState variables that are used is as follows:

### sampleISR()

- sysState.knobValue[0] - Volume value to right shift before waveform output
- sysState.knobValue[1] - Tone value to transform the waveform into the selected tone
- sysState.RX_Message - Received message values from other keyboards
- sysState.keys_down - Stores the key that is being pressed down

### playNotes

- sysState.octave - selected octave value to modify the frequency to selected octave
- sysState.keys_down - Key that has been pressed down

Alongside the sysState variables, two constant global arrays, sine[] and metronome[] are used in the playSampled and playMetronome functions, respectively, to produce the required sound. 

The synchronisation is achieved, as interrupts have a much higher frequency compared to the threads by a factor of 1000. Hence, even if the state variables are changed by the user, the period of the interrupt is so high that it will synchronise with a slight delay imperceptible to the user.

## Blocking dependencies and deadlock risk

As mutex is not used in this task due to it being an interrupt, there are no blocking dependencies and hence no deadlock risk. This is a self-contained output task, so it does not block any other task from running.
