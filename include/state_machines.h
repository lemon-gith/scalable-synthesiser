#ifndef STATE_MACHINES_H
#define STATE_MACHINES_H

#include "main.h"

// a state machine to ensure good UX for button presses
struct ButtonPress {
  uint8_t knob_name;
  enum State{OFF, ARMED, ON} state;

  // constructs a button state machine, given a name, starts OFF
  ButtonPress(uint8_t knob_name);

  // based on whether or not button is currently pressed,
  // will determine and return the next state
  // (if just state progression is desired, can ignore result)
  State nextState(bool isPressed);

  // just returns the current state, nothing else
  State getState();
};


#endif