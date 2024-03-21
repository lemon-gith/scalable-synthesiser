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

| Task | Functionality | Implementation Type (interrupt/thread) | Initiation Interval (ms)
Aka deadline | Worst-case execution time (ms) |
| --- | --- | --- | --- | --- |
| updateKeys | Reads and updates all input variables in system state | thread | 25.2 | 0.53425 

(19.233 ms for 36) |
| updateDisplay | Outputs UI onto display | thread | 100 | 16.778

(536.898 ms for 32) |
| decodeMessage | Loads oldest message in received queue into system state variable | thread | 25.2 | 0.009875

(0.3555 ms for 36)

(0.316 ms for 32) |
| CAN_TX_Task | Transmits oldest message in transmitting queue over CAN bus | thread | 60 | 0.01

(0.36 ms for 36)

(30 us for 3) |
| CAN_TX_ISR | Gives the semaphore to signal when a new transmission can be sent | interrupt | 60 | 0.0026667

(0.104 ms for 39)

(8 us for 3) |
| CAN_RX_ISR | Places a received message onto the received queue | interrupt | 25.2 | 0.99333

(35.76 ms for 36)

(2980 us for 3) |
| sampleISR | Based on the current system state (notes pressed, octave, tone, etc) output value to speaker | interrupt | 0.0455 | 0.0155

(496 us for 32) |

## Critical instance analysis

$$
\lceil \dfrac{100}{25.2} \rceil * 19.233
+ \lceil \dfrac{100}{100} \rceil * 16.778
+ \lceil \dfrac{100}{25.2} \rceil * 0.3555
+ \lceil \dfrac{100}{60} \rceil * 0.36
+ \lceil \dfrac{100}{60} \rceil * 0.104
+ \lceil \dfrac{100}{25.2} \rceil * 0.99333
+ \lceil \dfrac{100}{0.0455} \rceil * 0.0155
\le 100
$$

$$
4 * 19.233
+ 1 * 16.778
+ 4 * 0.3555
+ 2 * 0.36
+ 2 * 0.104
+ 4 * 0.99333
+ 2198 * 0.0155
\le 100
$$

## Dependency diagram
