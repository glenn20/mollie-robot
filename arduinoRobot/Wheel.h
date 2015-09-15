// -*- c++ -*-

#ifndef WHEEL_H
#define WHEEL_H

#include <Arduino.h>

#include "Encoder.h"
#include "Motor.h"
#include "PID.h"

// A "Wheel" is built from a DC motor controller ("Motor") and an "Encoder"
class Wheel {
public:
    Wheel(
	Motor&      motor,
	Encoder&    encoder,
	String      name = "Wheel",
	bool        usepid = true
	);

    void     initialise();
    void     enable();                  // Enable the wheel motor
    void     disable();                 // Disable the wheel motor
    int      setspeed( int speed );     // Set the target speed (in RPM)
    int      setspeed();                // Return the target speed
    int      speed();                   // Return the actual wheel speed
    int      power();                   // Return the motor power setting
    void     run( int speed ) { setspeed( speed ); };

    bool     Loop();                    // Called to update PID control

    Motor&   motor()   { return m_motor;   };
    Encoder& encoder() { return m_encoder; };
private:
    Motor&   m_motor;                   // The Motor controller
    Encoder& m_encoder;                 // The Encoder controller
    String   m_name;                    // A name for diagnostic printouts
    bool     m_usepid;
    MyPID    pid;                       // The PID controller
    int      m_speed;                   // The target speed for the wheel
    unsigned long m_tick;               // An internal loop counter
};

#endif // WHEEL_H

// Local Variables:
// c-basic-offset: 4
// End: 
