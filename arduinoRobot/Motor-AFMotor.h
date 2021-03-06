// -*- c++ -*-

#ifndef MOTOR_AFMOTOR_H
#define MOTOR_AFMOTOR_H

#include "AFMotor.h"

#include "Motor.h"

// Implementation of the Motor class using the AdaFruit Motor Shield.

class MotorAFMotor: public Motor {
public:
    MotorAFMotor();

private:
    // Setup the control and enable pins on the arduino
    void  doinitialise( int motornum );

    void  doclose();

    // Write the power setting to the (PWM) Enable pin on H-Bridge
    int   dosetpower( int power );   // Set the wheel power (-255->255)

private:
    AF_DCMotor *m_motor;
    bool       m_forwardp;
};

#endif //MOTOR_AFMOTOR_H

// Local Variables:
// c-basic-offset: 4
// End: 
