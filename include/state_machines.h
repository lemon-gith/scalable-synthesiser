#ifndef STATE_MACHINES_H
#define STATE_MACHINES_H
// state machine types just need to be defined in headers 
// to be included where needed


#include "main.h"


// a state machine to ensure good UX for button presses
struct ButtonPress {
  char knob_name;
  enum State{OFF, ARMED, ON} state;

  // constructs a button state machine, given a name, starts OFF
  ButtonPress(char knob_name){
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


// a state machine for joystick flick toggling
struct JoystickFlick {
  // base, up-right, right-up, right-down, etc.
  enum Direction{B, UR, RU, RD, DR, DL, LD, LU, UL} direction;
  // OFF for B, else ARMED for change, ON for no change
  enum State{OFF, ARMED, ON} state;
  // enumerates 5 possible joystick direction configs
  enum Config{UD, LR, UDLR, D8, D16};  // D16 not implemented yet

  // constructs a button state machine, given a name, starts OFF
  JoystickFlick(){
    this->direction = B;
    this->state = OFF;
  }

  // TODO: write this better, this seems table-like, think abt that
  
  // TODO: write a good description
  State nextState(Direction dir, Config config){
    if(dir == B){  // if not pressed, it's off...
      direction = B;
      return state = OFF;
    }
    else switch(config){
      case UD:
        if ((dir < RD || dir > LD)
        && (this->direction < RD || this->direction > LD))
          return this->state = ARMED;
        else
          return this->state = ON;
      if (dir != this->direction){
        this->direction = dir;
        return this->state = ARMED;
      }
      else  // if pressed and was already pressed, it is ON
        return this->state = ON;
  }
  }

  // just returns the current state, nothing else
  State getState(){
    return state;
  }

  // just returns the current direction, nothing else
  Direction getDirection(){
    return direction;
  }
};

/* idea for joystick fuzzy boundary SM:

state-driven, kinda, basically using a form of hysteresis?

0 (+1)
1 (<<)
10 (<<)
100 (+1)
101 (<<)
1010 (...)

then to get a sum of values :D

x = {binary hysteresis thing}
total = 0
for (sth i = 1; i < 0b10000; i <<= 1){
  total += x & i;
}

sth. like that, maybe?
*/

#endif