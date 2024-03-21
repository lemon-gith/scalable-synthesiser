# Task: Receiving CAN messages

This task sets up new memory space for variables used by its callees, then calls CAN_RX, which loads a received message into RX_Message_ISR. It then stores this message into the program’s internal queue, to be handled by the decoder, later.
## Task type and functions

## Measurement methodology

1. Initial Setup:
    1. Disable all other thread tasks and ISRs through preprocessor directives to isolate the `CAN_RX_ISR` function.
    2. Enabled loopback mode on the CAN module to allow for self-reception of messages, mimicking a full mailbox scenario without external CAN network traffic.
    3. Pre-filled the CAN mailboxes by executing `CAN_TX()` three times before the measurement loop to ensure the function won’t block while waiting for the queue to fill up.
    4. Configured a loop at the end of the `setup` function to execute the task 3 times and time it.
2. Worst-case configuration
    1. No changes needed to be made to this function for testing.

| Initiation Interval (ms) | Execution Time (ms) |
| --- | --- |
| - | 0.99333 |

## Task interval and execution time

## Data/resource use and synchronisation

## Blocking dependencies and deadlock risk
