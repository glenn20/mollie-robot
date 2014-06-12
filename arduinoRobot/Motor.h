// -*- c++ -*-

#ifndef MOTOR_H
#define MOTOR_H

#include "Encoder.h"

class MyPID {
public:
  MyPID()
    :      m_actualspeed( 0.0 ),
	   m_setspeed( 0.0 ),
	   m_output( 0.0 ),
	   m_pid(&m_actualspeed, &m_output, &m_setspeed, 0.5, 0, 0, DIRECT) {
    m_pid.SetOutputLimits(-50, 50);
    m_pid.SetMode(AUTOMATIC);
    m_pid.SetSampleTime( 500 );
  };
  
  boolean UpdatePID( double setspeed,
		     double measuredspeed,
		     double* output
    ) {
    m_actualspeed = measuredspeed;
    m_setspeed    = setspeed;
    if (m_pid.Compute()) {
      *output = m_output;
      return true;
    }
    return false;
  };
  
private:
  double   m_actualspeed;
  double   m_setspeed;
  double   m_output;
  PID      m_pid;
};

class Motor {
public:
  Motor(   int       controlpin1,
	   int       controlpin2,
	   int       enablepin,
	   Encoder&  encoder
    )
    :      m_controlpin1    ( controlpin1 ),
	   m_controlpin2    ( controlpin2 ),
	   m_enablepin      ( enablepin ),
	   m_encoder        ( encoder ),
	   pid              ( ),
	   m_motorDirection ( -1 ),
	   m_motorSpeed     ( 0 ),
	   m_motorEnabled   ( 1 ),
	   m_pwm            ( 0 ) 
    {};

  void     initialise();
  void     run( int speed, int direction );
  void     run( int speed );
  void     setspeed( int speed );
  void     setdirection( int direction );
  void     enable();
  void     disable();
  int      speed();
  void     setPWM( int pwm );
  int      pwm();
  boolean  Loop( String name );

  MyPID    pid;
private:
  Encoder& m_encoder;
  int      m_controlpin1;
  int      m_controlpin2;
  int      m_enablepin;
  int      m_motorDirection;
  int      m_motorSpeed;
  int      m_motorEnabled;
  int      m_pwm;
};

#endif MOTOR_H

// Local Variables:
// c-basic-offset: 2
// End: 
