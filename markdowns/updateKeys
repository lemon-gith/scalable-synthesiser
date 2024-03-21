# Task: updateKeys

The task reads all raw input values (from the keys, knobs, and joystick), processes them, and updates their values in the system state. If the module is a sender, it also places the latest message to transmit onto the transmitting queue.

## Task type and functions

The task is a thread since it should occur once every fixed sample interval. If it were triggered every time an input changed, it would result in an extremely high overhead with little noticable difference to the user. Processing of inputs such as skipping stages in the knob state diagram can help ease sample frequency requirements (since higher sample frequency to capture fast changes in the knob readings are not necessary when it can be assumed that an illegal transition means that, had an additional sample existed between two readings, it would have captured the in-between state).

This task calls the following functions:

- readKeys: reads input matrix rows [0,3] to find which values are low, and thus which keys are being pressed
- readKnobs: reads input matrix rows [3,6] to find raw values for the knob turning and pushing inputs
- xQueueSend: places a message onto the transmitting queue
- readRow (for joystick push): reads input matrix row 5, for value 2 which goes low when the joystick is pushed
- analogRead (for joystick axes): reads the raw values for variables X and Y of the joystick position
- navigate: updates values to be used in displaying the UI in accordance with joystick movement and press
- calcJoy: returns principal movement direction of joystick and press/release state

## Task interval and execution time

The initiation interval set to 25.2ms.

- Calculating the execution time
    - Explain measurement method

To accurately characterise the `updateKeys` task, we implemented a series of steps to isolate the task and measure its performance under worst-case scenarios. 

1. Disable other tasks threads and ISRs using preprocessors directions to ensure this is the only running task during measurement.
2. Configured the `updateKeysTask` function for a worst-case execution scenario by modifying it to simulate a key press for of the 12 keys on every iteration.
3. Adjusted the function to bypass `vTaskDelayUntil` to allow it to execute continuously without blocking and removed the while(1) loop to ensure 1 iteration per function run.
4. Increased the size of `msgOutQ` to 384 to prevent the task from blocking due to a full queue, enabling it run an iteration of 12 key presses.
5. Measured the execution time over 32 iterations using the `micros()` function.

| Initiation Interval (ms) | Execution Time (ms) |
| --- | --- |
| 25.200 | 0.53426 |

## Data/resource use and synchronisation

This task accesses the following global variables:

- sysState.TX_Message
- sysState.isSender
- sysState.knobValues
- sysState.octave
- sysState.keyStrings
- sysState.keys_down
- sysState.knobPushes
- sysState.next_state (joystick state machine): increments the state of the joystick

It also accesses the following additional global variables when calling function navigate:

- sysState.menuState
- sysState.isSelected
- sysState.metOnState
- sysState.met
- sysState.dotLocation
- sysState.octave

The synchronisation is managed by taking sysState.mutex. This means that no other tasks can access sysState until reading/writing is over. The mutexes are released directly after read/write operations occur, and only read/write operations happen while the mutex is taken. This minimises the amount of time during which the mutex could potentially be blocking other tasks.

This task also uses the resource msgOutQ, and will push between 0-12 messages onto this queue. Synchronisation is managed by FreeRTOS.

## Blocking dependencies and deadlock risk

updateKeys blocks all other functions that use sysState when it takes the mutex. However, there is no risk of deadlock, since all functions that locks a mutex will always unlock it, and no blocking statements occur while the mutex is locked (since mutexes are only ever locked while variable accessing occurs, and not for any variable processing).

updateKeys is blocked by CAN_TX_Task. If the queue is not emptied fast enough (i.e. insufficient capacity), updateKeys cannot run again until items are removed.
