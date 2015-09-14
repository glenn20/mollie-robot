// -*- c++ -*-

#ifndef MOTOR_AFMOTOR_H
#define MOTOR_AFMOTOR_H

#include "AFMotor.h"

#include "Motor.h"

// Implementation of the Motor class using the AdaFruit Motor Shield.

// Constructor - uses 3 Arduino pins for each motor:
//  - Control Pin 1    (Digital Output - sets motor direction)
//  - Control Pin 2    (Digital Output - sets motor direction)
//  - Enable Pin       (PWM - sets motor power levels (voltage))

class MotorAFMotor: public Motor {
public:
  MotorAFMotor( int motornum );

private:
  // Setup the control and enable pins on the arduino
  void  doinitialise();

  // Write the power setting to the (PWM) Enable pin on H-Bridge
  int   dosetpower( int power );   // Set the wheel power (0-255)

private:
  int        m_motornum;
  bool       m_forwardp;
  AF_DCMotor m_motor;
};

#endif //MOTOR_AFMOTOR_H

// Local Variables:
// c-basic-offset: 2
// End: 
