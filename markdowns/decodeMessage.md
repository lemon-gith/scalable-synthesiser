# Task: decodeMessage

The task pops the oldest message from the receiving queue, and writes it into the system state’s “last received message” variable.

## Task type and functions

This task is a thread, but it is triggered by the availability of messages on the queue rather than a fixed interval. It works in tandem with CAN_RX_ISR and is analogous to CAN_TX_Task; the ISR means that all messages received will be placed into the queue since it must finish, while the decoding thread can block and yield to other tasks while waiting for messages, thereby freeing up the CPU.

This task calls the following functions:

- xQueueReceive: copies a message from the receiving queue into a buffer

## Task interval and execution time

## Data/resource use and synchronisation

This task uses the resource msgInQ, which is the queue of all received messages. The synchronisation of this queue is managed by FreeRTOS.

The task also writes to global variable sysState.RX_Message. This is accessed in a thread-safe manner by using a mutex to block other accesses. Only the copying of the local message buffer into this variable is contained within the mutex to reduce the length of blocking time.

## Blocking dependencies and deadlock risk

decodeMessage blocks all other tasks that use sysState when it takes the mutex, and is in turn blocked when other tasks lock the same mutex. However, there is no risk of deadlock, since all functions that locks a mutex will always unlock it, and no blocking statements occur while the mutex is locked (since mutexes are only ever locked while variable accessing occurs, and not for any variable processing).

decodeMessage does not block while waiting for new items on the queue, and has no other dependencies. Therefore, there is no risk of deadlock (since a dependency cycle needs at least one connection of each of the “is blocked by” and “is blocking” types on every graph node).