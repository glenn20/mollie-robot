// -*- c++ -*-

#include "Motor-AFMotor.h"

#include <Arduino.h>

// Implementation of the Motor class using the AdaFruit Motor Shield.

// Constructor - uses 3 Arduino pins for each motor:
//  - Control Pin 1    (Digital Output - sets motor direction)
//  - Control Pin 2    (Digital Output - sets motor direction)
//  - Enable Pin       (PWM - sets motor power levels (voltage))
MotorAFMotor::MotorAFMotor( int motornum )
  : m_motornum  ( motornum ),
    m_forwardp  ( false ),
    m_motor     ( motornum, MOTOR12_1KHZ )
{
}

// Initialise the 
void MotorAFMotor::doinitialise()
{
  setpower( 0 );
  
  Serial.print( "AFMotor::initialise: motornum = " );
  Serial.print( m_motornum );
  Serial.print( " " );
}

// Implementation of the abstract base method to set the motor power
// Write the power (voltage) setting (-255->255) to the PWM Enable Pin.
int MotorAFMotor::dosetpower( int power )
{
  bool forwardp = (power >= 0);
  if (m_forwardp != forwardp) {
    // If we have changed direction - set the H-bridge direction control
    m_forwardp = forwardp;
    m_motor.run( (forwardp) ? FORWARD : BACKWARD );
  }
  m_motor.setSpeed( power );
  return power;
}

// Local Variables:
// c-basic-offset: 2
// End: 