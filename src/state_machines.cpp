#include "state_machines.h"
// TODO: write up other state machine types


struct ButtonPress {
  uint8_t knob_name;
  enum State{OFF, ARMED, ON} state;

  ButtonPress(uint8_t knob_name){
    this->knob_name = knob_name;
    this->state = OFF;
  }

  State nextState(bool isPressed){
    if(!isPressed)  // if not pressed, it's off...
      return this->state = OFF;  // will it work without `this->`?
    else 
      if (this->state == OFF)  // if pressed and was OFF, must trigger
        return this->state = ARMED;
      else  // if pressed and was already pressed, it is ON
        return this->state = ON;
  }
  
  State getState(){
    return state;
  }
};