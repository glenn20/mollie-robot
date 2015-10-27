// -*- c++ -*-

#ifndef PID_H
#define PID_H

#include <PID_v1.h>

class MyPID {
public:
    MyPID( float Kp, float Ki, float Kd,
	   double min = 0, double max = 255, int sampletime_ms = 100
	)
	: m_actual( 0.0 ),
	  m_target( 0.0 ),
	  m_output( 0.0 ),
	  m_pid( &m_actual, &m_output, &m_target, Kp, Ki, Kd, DIRECT )
	{
	    m_pid.SetOutputLimits( min, max );
	    m_pid.SetMode(AUTOMATIC);
	    m_pid.SetSampleTime( sampletime_ms );
	};
    
    bool UpdatePID( double  target,
		    double  actual
	) {
	m_actual  = actual;
	m_target  = target;
	if (m_pid.Compute()) {
	    return true;
	}
	return false;
    };

    double output() {
	return m_output;
    };

    void setoutput( double output ) {
	m_output = output;
    }

    void setlimits( double min, double max ) {
	m_pid.SetOutputLimits( min, max );
    };

    void setPID( double Kp, double Ki, double Kd ) {
	m_pid.SetTunings( Kp, Ki, Kd );
    };

    double Kp() { return m_pid.GetKp(); };
    double Ki() { return m_pid.GetKi(); };
    double Kd() { return m_pid.GetKd(); };
  
private:
    double   m_actual;
    double   m_target;
    double   m_output;
    PID      m_pid;
};

#endif //PID_H

// Local Variables:
// c-basic-offset: 4
// End: 
