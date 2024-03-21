# ES_Synth Report

Group Peaky Blinders’ report for the ES_synth project.
## Table of Contents
- [updateKeys Task](https://github.com/MITeo21/ES-synth/blob/master/markdowns/updateKeys.md)
- [updateDisplay Task](https://github.com/MITeo21/ES-synth/blob/master/markdowns/updateDisplay.md)
- [decodeMessage Task](https://github.com/MITeo21/ES-synth/tree/master/markdowns/decodeMessage.md)
- [CAN_TX_TASK](https://github.com/MITeo21/ES-synth/blob/master/markdowns/CAN_TX_TASK.md)
- [CAN_TX_ISR](https://github.com/MITeo21/ES-synth/blob/master/markdowns/CAN_TX_ISR.md)
- [CAN_RX_ISR](https://github.com/MITeo21/ES-synth/tree/master/markdowns/CAN_RX_ISR.md)
- [sampleISR](https://github.com/MITeo21/ES-synth/blob/master/markdowns/sampleISR.md)


## Advanced features implemented

- [Octaves](https://github.com/MITeo21/ES-synth/blob/master/markdowns/octaves.md)
- [UI](https://github.com/MITeo21/ES-synth/blob/master/markdowns/UI.md)
- [Polyphony](https://github.com/MITeo21/ES-synth/blob/master/markdowns/polyphony.md)
- [Tones](https://github.com/MITeo21/ES-synth/blob/master/markdowns/tones.md)
- [Metronome](https://github.com/MITeo21/ES-synth/blob/master/markdowns/Metronome.md)
- [Video](https://www.youtube.com/watch?v=aVbcZlZeOfQ&feature=youtu.be)

## Task categorisations and characterisations

| Task | Functionality | Implementation Type (interrupt/thread) | Initiation Interval (ms) Aka deadline | Worst-case execution time (ms) |
| --- | --- | --- | --- | --- |
| updateKeys | Reads and updates all input variables in system state | thread | 25.2 | 0.53425 (17096 us for 32) |
| updateDisplay | Outputs UI onto display | thread | 100 | 16.778 (536898 us for 32) |
| decodeMessage | Loads oldest message in received queue into system state variable | thread |  | 0.009875 (316 us for 32) note: we had to fill up the msgin queue to be able to run the function otherwise the queuereceive blocked the function |
| CAN_TX_Task | Transmits oldest message in transmitting queue over CAN bus | thread |  | 0.01 (30 us for 3) <ul><li> ran the test 3 times</li></ul> |
| CAN_TX_ISR | Gives the semaphore to signal when a new transmission can be sent | interrupt |  | 0.0026667 (8 us for 3) <ul><li> had to change the give from ISR to just give </li><li> ran the test for 3 times</li> |
| CAN_RX_ISR | Places a received message onto the received queue | interrupt |  | 0.99333 (2980 us for 3) <ul> <li> had to set loopback to true </li><li> to make 3 calls of CAN_TX to fill up the mailbox</li><li> had to change the queuesendfromISR to just queuesend</li><li> ran the test for 3 times</li> </ul> |
| sampleISR | Based on the current system state (notes pressed, octave, tone, etc) output value to speaker | interrupt | 0.0455 | 0.0155 (496 us for 32) |

## Critical instance analysis


```math
\lceil \dfrac{}{25.2} \rceil * 0.53425
+ \lceil \dfrac{}{100} \rceil * 16.778
+ \lceil \dfrac{}{} \rceil * 0.009875
+ \lceil \dfrac{}{} \rceil * 0.01
+ \lceil \dfrac{}{} \rceil * 0.0026667
+ \lceil \dfrac{}{} \rceil * 0.99333
+ \lceil \dfrac{}{0.0455} \rceil * 0.00155
\le
```

## List of resources and allocation


## Dependency diagram
