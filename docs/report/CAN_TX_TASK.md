# Task: CAN_TX_Task

The function pops the oldest message to be transmitted into a local buffer, takes the transmitting semaphore, and calls the CAN message transmitting function.

## Task type and functions

This task is a thread, but it is triggered by the availability of messages on the queue rather than a fixed interval, and also must wait for the CAN hardware to be ready. It works in tandem with CAN_TX_ISR and is analogous to decodeMessage; this thread can block and yield to other tasks while waiting for messages from the rest of the system, thereby freeing up the CPU. Then, when ready to transmit, it will attempt to take the semaphore; therefore blocking it if there are already 3 messages waiting to be transmitted, and again yielding to other tasks. This prevents the rest of the system stalling if the CAN hardware is busy.

This task calls the following functions:

- xQueueReceive: copies a message from the queue of messages ready to transmit into a buffer
- CAN_TX: Sends a 32 bit message on the CAN bus

## Task interval and execution time
For the initiation interval, the frequency of initiation that prevents `msgOutQ` from filling up was required. `msgOutQ` is pushed by `updateKeysTask` which has an interval of 20ms and generation of 12 messages per interval at the worst case. As the queue is 36 items long, only 3 iterations of `updateKeysTask` are required to fill it up, which gives

## Measurement Methodology

1. Disabled all other thread tasks and ISRs through preprocessor directives to isolate the `CAN_TX_Task`.
2. Configured a loop at the end of the `setup` function to execute the task 3 times.
3. Increased the size of `msgOutQ` to 1000 to prevent the task from blocking.
4. Limited the execution loop to 3 iterations in alignment with the CAN hardware’s three mailbox slots, to prevent blocking due to the semaphore limit.

```math
3 \times 20ms = 60ms
```
Therefore, in order to empty `msgOutQ`, 36 items need to be removed every 60ms, which is the initation interval.

| Initiation Interval (ms) | Execution Time (ms) |
| --- | --- |
| - | 0.01 |


## Data/resource use and synchronisation

This task uses the resource msgOutQ, which is the queue of all messages ready to transmit. The synchronisation of this queue is managed by FreeRTOS.

## Blocking dependencies and deadlock risk

This task does not block while waiting to receive messages on the queue, and only blocks itself when taking CAN_TX_Semaphore three times in a row without any releases from CAN_TX_ISR. The only risk of deadlock occurring is if the CAN bus stops transmitting for any reason.
