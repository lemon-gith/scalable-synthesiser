# Task: CAN_TX_Task

The function pops the oldest message to be transmitted into a local buffer, takes the transmitting semaphore, and calls the CAN message transmitting function.

## Task type and functions

This task is a thread, but it is triggered by the availability of messages on the queue rather than a fixed interval, and also must wait for the CAN hardware to be ready. It works in tandem with CAN_TX_ISR and is analogous to decodeMessage; this thread can block and yield to other tasks while waiting for messages from the rest of the system, thereby freeing up the CPU. Then, when ready to transmit, it will attempt to take the semaphore; therefore blocking it if there are already 3 messages waiting to be transmitted, and again yielding to other tasks. This prevents the rest of the system stalling if the CAN hardware is busy.

This task calls the following functions:

- xQueueReceive: copies a message from the queue of messages ready to transmit into a buffer
- CAN_TX: Sends a 32 bit message on the CAN bus

## Task interval and execution time


## Data/resource use and synchronisation

This task uses the resource msgOutQ, which is the queue of all messages ready to transmit. The synchronisation of this queue is managed by FreeRTOS.

## Blocking dependencies and deadlock risk

This task does not block while waiting to receive messages on the queue, and only blocks itself when taking CAN_TX_Semaphore three times in a row without any releases from CAN_TX_ISR. The only risk of deadlock occurring is if the CAN bus stops transmitting for any reason.
