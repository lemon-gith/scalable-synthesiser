CAN_TX_ISR

# Task: CAN_TX_ISR

Gives the semaphore to signal when a new transmission can be sent, based on mailbox status.

## Task type and functions

This task is an interrupt since it is attached to the CAN transmitting routine via CAN_RegisterTX_ISR. It needs to trigger every time one of the 3 CAN transmitting mailboxes becomes free to send another message.

There are no other functions allocated to this task apart from releasing one count of the CAN_TX_Semaphore.

## Task interval and execution time

## Data/resource use and synchronisation

This tasks does not use any data/resources. It manages synchronisation by releasing a count of CAN_TX_Semaphore every time it runs.

## Blocking dependencies and deadlock risk

This task is not blocked by any other tasks, and does not block any other tasks directly. It prevents deadlock from occurring by unlocking the CAN_TX_Semaphore which CAN_TX_Task locks.

