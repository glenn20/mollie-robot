// -*- c++ -*-

#ifndef WHEEL_H
#define WHEEL_H

#include <Arduino.h>

#include "Encoder.h"
#include "Motor.h"
#include "PID2.h"

// A "Wheel" is built from a DC motor controller ("Motor") and an "Encoder"
class Wheel {
public:
    Wheel(
	Motor&      motor,
	Encoder&    encoder,
	String      name = "Wheel"
	);

    void     initialise();
    void     enable();                  // Enable the wheel motor
    void     disable();                 // Disable the wheel motor
    float    setspeed( float speed );   // Set the target speed (in RPM)
    float    setspeed();                // Return the target speed
    float    speed();                   // Return the actual wheel speed
    int      setpower( int power );	// Set the motor power
    float    power();                   // Return the motor power setting
    void     stop();                    // Stop the wheel
    void     run( float speed ) { setspeed( speed ); };

    bool     Loop();                    // Called to update PID control

    Motor&   motor()   { return m_motor;   };
    Encoder& encoder() { return m_encoder; };
    
private:
    int       updatePid( int power, float targetValue, float currentValue);

private:
    Motor&   m_motor;                   // The Motor controller
    Encoder& m_encoder;                 // The Encoder controller
    String   m_name;                    // A name for diagnostic printouts
    MyPID2   pid;                       // The PID controller
    float    m_setspeed;                // The target speed for the wheel
    bool     m_setspeedp;		// Manage for constant speed?
    unsigned long m_tick;               // An internal loop counter
};

#endif // WHEEL_H

// Local Variables:
// c-basic-offset: 4
// End: 
