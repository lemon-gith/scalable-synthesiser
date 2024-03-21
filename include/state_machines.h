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
  // reduced for use
  char direction;  // only b, u, d, l, r, are valid
  
  // base, up-right, right-up, right-down, etc.
  // enum Direction{B, UR, RU, RD, DR, DL, LD, LU, UL} direction;
  // TODO: ^ rewrite to give easy, numerical UD and LR splits
  
  // OFF for B, else ARMED for change, ON for no change
  enum State{OFF, ARMED, ON} state;
  // enumerates 5 possible joystick direction configs
  enum Config{UD, LR, UDLR, D8, D16};  // D16 not implemented yet

  // constructs a button state machine, given a name, starts OFF
  JoystickFlick(){
    direction = 'b';
    state = OFF;
  }

  void toDirection(char dir){
    direction = dir;
  }

  // TODO: write this better, this seems table-like, think abt that
  // maybe use different encoding for directions, two chars?
  
  /* currently deprecated for simpler implementation
  // given the current direction and current joystick config mode
  // will determine and return the next state
  State nextState(Direction dir, Config config){
    if(dir == B){  // if at Base, do nothing
      direction = B;
      return state = OFF;
    }
    else if((direction == B) && (dir != B)){
      direction = dir;
      return state = ARMED;
    }  // early returns for readability
    else switch(config){
      case UD:
        // NOT IMPLEMENTED
        break;
      case LR:
        // NOT IMPLEMENTED
        break;
      case UDLR:
        switch (dir){
          case UR: case UL:  // either of these (fall-through)
            if ((direction == UR) || (direction == UL))
              state = ON;
            else
              state = ARMED;
            break;
          case RU: case RD:
            if ((direction == RU) || (direction == RD))
              state = ON;
            else
              state = ARMED;
            break;
          case DR: case DL:
            if ((direction == DR) || (direction == DL))
              state = ON;
            else
              state = ARMED;
            break;
          case LD: case LU:
            if ((direction == LD) || (direction == LU))
              state = ON;
            else
              state = ARMED;
            break;
          default:
            return state = OFF;
        }
        break;
      case D8:
        // NOT IMPLEMENTED
        break;
      case D16:
        // NOT IMPLEMENTED
        break;
    }
    direction = dir;
    return state;
  }
  */
  
  State next_state(char dir){
    if(dir == 'b')
      state = OFF;  // if at Base, do nothing
    else if(direction != dir)
      state = ARMED;
    else
      state = ON;

    direction = dir;  // cast to enum
    return state;
  }
  // just returns the current state, nothing else
  State getState(){
    return state;
  }

  // just returns the current direction, nothing else
  char getDirection(){
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