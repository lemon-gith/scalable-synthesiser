# Task: CAN_TX_ISR

Gives the semaphore to signal when a new transmission can be sent, based on mailbox status.

## Task type and functions

This task is an interrupt since it is attached to the CAN transmitting routine via CAN_RegisterTX_ISR. It needs to trigger every time one of the 3 CAN transmitting mailboxes becomes free to send another message.

There are no other functions allocated to this task apart from releasing one count of the CAN_TX_Semaphore.

## Task interval and execution time
The frequency of this interrupts execution is dependent on `CAN_TX_TASK`, as each execution of `CAN_TX_TASK` will execute this ISR. As shown in [CAN_TX_TASK](https://github.com/MITeo21/ES-synth/blob/master/markdowns/CAN_TX_TASK.md), the worst case execution time is 36 iterations every 60ms, and so will be the same for this. In the worst case, 3 CAN mailboxes are used, which are also full at the start of critical instant, which is added and gives the worst case execution time of 39 itrations per 60ms. The accumulated execution time is
```math
0.0026667 \times 39 = 104 \micro s
```

##Measuring Methodology

1. Initial Setup:
    1. Disabled all other thread tasks and ISRs through preprocessor directives to isolate the `CAN_TX_ISR` function.
    2. Increased the size of `msgOutQ` to 1000 to prevent the task from blocking.
    3. Configured a loop at the end of the `setup` function to execute the task 3 times.
    4. Limited the execution loop to 3 iterations in alignment with the CAN hardware’s three mailbox slots, to prevent blocking due to the semaphore limit.
2. Function configuration
    1. Adjusted the semaphore operations to use `xSemaphoreGive` suited for non-ISR contexts, to setup the function for testing
  
| Initiation Interval (ms) | Execution Time (ms) |
| --- | --- |
| - | 0.0026667 |
	

## Data/resource use and synchronisation

This tasks does not use any data/resources. It manages synchronisation by releasing a count of CAN_TX_Semaphore every time it runs.

## Blocking dependencies and deadlock risk

This task is not blocked by any other tasks, and does not block any other tasks directly. It prevents deadlock from occurring by unlocking the CAN_TX_Semaphore which CAN_TX_Task locks.

