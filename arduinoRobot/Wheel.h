// -*- c++ -*-

#ifndef WHEEL_H
#define WHEEL_H

#include <Arduino.h>

#include "Encoder.h"
#include "Motor.h"
#include "PID.h"

#define PID_NONE          0
#define PID_STARTING      1
#define PID_HIGHPOWER     2
#define PID_LOWPOWER      3
#define PID_PIDCONTROL    4

// A "Wheel" is built from a DC motor controller ("Motor") and an "Encoder"
class Wheel {
public:
    Wheel(
	Motor&      motor,
	Encoder&    encoder,
	MyPID&      pid,
	String      name = "Wheel"
	);

    void     initialise();
    void     close();
    void     enable();                  // Enable the wheel motor
    void     disable();                 // Disable the wheel motor
    float    setspeed( float speed );   // Set the target speed (in RPM)
    float    setspeed();                // Return the target speed
    float    speed();                   // Return the actual wheel speed
    int      setpower( int power );	// Set the motor power
    float    power();                   // Return the motor power setting
    int      count();                   // Return the encoder counts
    void     stop();                    // Stop the wheel
    void     run( float speed ) { setspeed( speed ); };

    bool     Loop();                    // Called to update PID control

    Motor&   motor()   { return m_motor;   };
    Encoder& encoder() { return m_encoder; };
    MyPID&   pid()     { return m_pid;     };
    
private:
    Motor&        m_motor;              // The Motor controller
    Encoder&      m_encoder;            // The Encoder controller
    MyPID&        m_pid;                // The PID controller
    String        m_name;               // A name for diagnostic printouts
    unsigned long m_tick;               // An internal loop counter
    float         m_setspeed;           // The target speed for the wheel
    unsigned long m_starttime;
    unsigned long m_startcount;
    double        m_pidpower;
    int           m_controlstate;
};

#endif // WHEEL_H

// Local Variables:
// c-basic-offset: 4
// End: 
