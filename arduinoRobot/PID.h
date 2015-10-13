// -*- c++ -*-

#ifndef PID_H
#define PID_H

#include <PID_v1.h>

class MyPID {
public:
    MyPID( float Kp, float Ki, float Kd,
	   float min, float max, int sampletime_ms
	)
	: m_actual( 0.0 ),
	  m_target( 0.0 ),
	  m_output( 0.0 ),
	  m_Kp    ( Kp ),
	  m_Ki    ( Ki ),
	  m_Kd    ( Kd ),
	  m_pid( &m_actual, &m_output, &m_target, m_Kp, m_Ki, m_Kd, DIRECT )
	{
	    m_pid.SetOutputLimits( min, max );
	    m_pid.SetMode(AUTOMATIC);
	    m_pid.SetSampleTime( sampletime_ms );
	};
    
    bool UpdatePID( double  target,
		    double  actual,
		    double* output
	) {
	m_actual  = actual;
	m_target  = target;
	if (output != NULL) {
	    m_output  = *output;
	}
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
    double   m_actual;
    double   m_target;
    double   m_output;
    double   m_Kp;
    double   m_Ki;
    double   m_Kd;
    PID      m_pid;
};

#endif //PID_H

// Local Variables:
// c-basic-offset: 4
// End: 
