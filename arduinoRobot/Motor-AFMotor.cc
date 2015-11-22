// -*- c++ -*-

#include "Motor-AFMotor.h"

#include <Arduino.h>

// Implementation of the Motor class using the AdaFruit Motor Shield.

// Constructor
MotorAFMotor::MotorAFMotor()
    : m_motor     ( NULL ),
      m_forwardp  ( false )
{
}

// Initialise the motor
void MotorAFMotor::doinitialise( int motornum )
{
    // If already initialise - de-initialise
    close();

    // Prefer not to allocate from heap in an embedded system, but the
    // AF_DCMotor class leaves us no other option when we need to configure
    // after instantiation.
    m_motor = new AF_DCMotor( motornum, MOTOR12_64KHZ );
	
    m_motor->setSpeed( 0 );
    m_motor->run( m_forwardp ? FORWARD : BACKWARD );
    setpower( 0 );
 
    Serial.print( F("AFMotor::initialise: motornum = ") );
    Serial.println( motornum );
}

// Initialise the motor
void MotorAFMotor::doclose()
{
    if (m_motor != NULL) {
	free( m_motor );
	m_motor = NULL;
    }
}

// Implementation of the abstract base method to set the motor power
// Write the power (voltage) setting (-255->255) to the PWM Enable Pin.
int MotorAFMotor::dosetpower( int power )
{
    if (power < -255)
	power = -255;
    if (power > 255)
	power = 255;
    if (m_motor == NULL) {
	return power;
    }
    bool forwardp = (power >= 0);
    if (m_forwardp != forwardp) {
	// If we have changed direction - set the H-bridge direction control
	m_forwardp = forwardp;
	m_motor->run( forwardp ? FORWARD : BACKWARD );
    }
    m_motor->setSpeed( m_forwardp ? power : -power );
    return power;
}

// Local Variables:
// c-basic-offset: 4
// End: 
