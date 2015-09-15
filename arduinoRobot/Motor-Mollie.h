// -*- c++ -*-

#ifndef MOTOR_MOLLIE_H
#define MOTOR_MOLLIE_H

#include "Motor.h"

// Implementation of the Motor class using Arduino pins to control an external
// H-Bridge. This is how Mollie's robot works.

// Constructor - uses 3 Arduino pins for each motor:
//  - Control Pin 1    (Digital Output - sets motor direction)
//  - Control Pin 2    (Digital Output - sets motor direction)
//  - Enable Pin       (PWM - sets motor power levels (voltage))

class MotorMollie: public Motor {
public:
    MotorMollie( int       controlpin1,
		 int       controlpin2,
		 int       enablepin
	);

private:
    // Setup the control and enable pins on the arduino
    void  doinitialise();

    // Write the power setting to the (PWM) Enable pin on H-Bridge
    int   dosetpower( int power );   // Set the wheel power (0-255)

private:
    int      m_controlpin1;
    int      m_controlpin2;
    int      m_enablepin;
    bool     m_forwardp;
};

#endif //MOTOR_MOLLIE_H

// Local Variables:
// c-basic-offset: 4
// End: 
