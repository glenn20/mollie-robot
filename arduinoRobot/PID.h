// -*- c++ -*-

#ifndef PID_H
#define PID_H

#include <PID_v1.h>

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
  
  bool UpdatePID( double setspeed,
		  double measuredspeed,
		  double* output = NULL
    ) {
    m_actualspeed = measuredspeed;
    m_setspeed    = setspeed;
    if (m_pid.Compute()) {
      if (output != NULL) {
	*output = m_output;
      }
      return true;
    }
    return false;
  };

  double output() {
    return m_output;
  }
  
private:
  double   m_actualspeed;
  double   m_setspeed;
  double   m_output;
  PID      m_pid;
};

#endif //PID_H

// Local Variables:
// c-basic-offset: 2
// End: 
