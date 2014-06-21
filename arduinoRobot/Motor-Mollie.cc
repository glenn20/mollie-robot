// -*- c++ -*-

#include "Motor-Mollie.h"

#include <Arduino.h>

// Implementation of the Motor class using Arduino pins to control an external
// H-Bridge. This is how Mollie's robot works.

// Constructor - uses 3 Arduino pins for each motor:
//  - Control Pin 1    (Digital Output - sets motor direction)
//  - Control Pin 2    (Digital Output - sets motor direction)
//  - Enable Pin       (PWM - sets motor power levels (voltage))
MotorMollie::MotorMollie(
  int       controlpin1,
  int       controlpin2,
  int       enablepin
  )
  : m_controlpin1  ( controlpin1 ),
    m_controlpin2  ( controlpin2 ),
    m_enablepin    ( enablepin ),
    m_forwardp     ( false )
{
}

// Initialise the 
void MotorMollie::initialise()
{
  // Initialise the arduino pins to control the DC motor
  pinMode( m_controlpin1, OUTPUT );
  pinMode( m_controlpin2, OUTPUT );
  pinMode( m_enablepin,   OUTPUT );
  disable();
  setpower( 0 );
  
  Serial.print( "Motor::initialise: Pins = " );
  Serial.print( m_controlpin1 );
  Serial.print( " " );
  Serial.print( m_controlpin2 );
  Serial.print( " " );
  Serial.print( m_enablepin );
  Serial.println( " " );
}

// Implementation of the abstract base method to set the motor power
// Write the power (voltage) setting (0-255) to the PWM Enable Pin.
int MotorMollie::dosetpower( int power )
{
  bool forwardp = (power >= 0);
  if (m_forwardp != forwardp) {
    // If we have changed direction - set the H-bridge direction control
    m_forwardp = forwardp;
    digitalWrite( m_controlpin1, (forwardp) ? HIGH : LOW );
    digitalWrite( m_controlpin2, (forwardp) ? LOW  : HIGH);
    delay(1);
  }
  analogWrite( m_enablepin, power );
  return power;
}

// Local Variables:
// c-basic-offset: 2
// End: 
