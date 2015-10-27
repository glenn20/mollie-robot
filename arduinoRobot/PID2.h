// -*- c++ -*-

#ifndef PID2_H
#define PID2_H

class MyPID2 {
public:
    MyPID2(
	double Kp, double Ki, double Kd,
	double min = 0, double max = 255, int sampletime_ms = 100
	)
	: m_output    ( 0.0 ),
	  m_Kp        ( Kp ),
	  m_Ki        ( Ki ),
	  m_Kd        ( Kd ),
	  m_min       ( min ),
	  m_max       ( max ),
	  m_lasterror ( 0 ),
	  m_sampletime_ms( sampletime_ms ),
	  m_lastupdatetime( 0 )
	{
	};

    bool UpdatePID(
	double   target,
	double   actual
	) {
	int t = millis();
	if (t - m_lastupdatetime >= m_sampletime_ms) {
	    m_lastupdatetime = t;
	    double error  = target - actual; 
	    double x      = ((m_Kp * error) +
			     (m_Kd * (error - m_lasterror)));
	    m_lasterror   = error;
	    x = (x < m_min ? m_min :
		 x > m_max ? m_max :
		 x);
	    m_output = x;
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

    void setPID( double Kp, double Ki, double Kd ) {
	m_Kp = Kp; m_Ki = Ki; m_Kd = Kd;
    };

    double Kp() { return m_Kp; };
    double Ki() { return m_Ki; };
    double Kd() { return m_Kd; };

    void reset() { m_lasterror = 0.0; };

private:
    double    m_output;
    double    m_Kp;			// PID proportionality constant
    double    m_Ki;
    double    m_Kd;			// PID derivative constant
    double    m_min;
    double    m_max;
    double    m_lasterror;		// Save the last error - for next PID
    unsigned long m_sampletime_ms;	// PID update time in milliseconds
    unsigned long m_lastupdatetime;
};

#endif //PID2_H

// Local Variables:
// c-basic-offset: 4
// End: 
