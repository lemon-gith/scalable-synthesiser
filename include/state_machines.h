#ifndef STATE_MACHINES_H
#define STATE_MACHINES_H
// state machine types just need to be defined in headers 
// to be included where needed


#include "main.h"


// a state machine to ensure good UX for button presses
struct ButtonPress {
  uint8_t knob_name;
  enum State{OFF, ARMED, ON} state;

  // constructs a button state machine, given a name, starts OFF
  ButtonPress(uint8_t knob_name){
    this->knob_name = knob_name;
    this->state = OFF;
  }

  // based on whether or not button is currently pressed,
  // will determine and return the next state
  // (if just state progression is desired, can ignore result)
  State nextState(bool isPressed){
    if(!isPressed)  // if not pressed, it's off...
      return this->state = OFF;  // will it work without `this->`?
    else 
      if (this->state == OFF)  // if pressed and was OFF, must trigger
        return this->state = ARMED;
      else  // if pressed and was already pressed, it is ON
        return this->state = ON;
  }

  // just returns the current state, nothing else
  State getState(){
    return state;
  }
};


#endif