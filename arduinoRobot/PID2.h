// -*- c++ -*-

#ifndef PID2_H
#define PID2_H

class MyPID2 {
public:
    MyPID2(
	float Kp	    = 0.0,	// Scale factor to apply
	float Ki            = 0.0,	// PID proportionality constant
	float Kd            = 0.0,	// PID derivative constant
	float min           = -255,
	float max           =  255,
	int   sampletime_ms = 100	// Update period - in seconds
	) : m_Kp        ( Kp ),
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
	double   targetValue,
	double   currentValue,
	double*  output
	)
	{
	    int t = millis();
	    if (t - m_lastupdatetime >= m_sampletime_ms) {
		m_lastupdatetime = t;
		double error  = targetValue - currentValue; 
		double x      = ((m_Kp * error) +
				 (m_Kd * (error - m_lasterror)));
		m_lasterror   = error;
		x = (x < m_min ? m_min :
		     x > m_max ? m_max :
		     x);
		*output = x;
		return true;
	    }
	    return false;
	};

    void setKp( float Kp ) { m_Kp = Kp; };
    void setKi( float Ki ) { m_Ki = Ki; };
    void setKd( float Kd ) { m_Kd = Kd; };
    void setK( float Kp, float Ki, float Kd ) { setKp( Kp ); setKi( Ki ); setKd( Kd ); };

    float Kp() { return m_Kp; };
    float Ki() { return m_Ki; };
    float Kd() { return m_Kd; };

    void reset() { m_lasterror = 0.0; };

private:
    float    m_Kp;			// PID proportionality constant
    float    m_Ki;
    float    m_Kd;			// PID derivative constant
    float    m_min;
    float    m_max;
    float    m_lasterror;		// Save the last error - for next PID
    unsigned long m_sampletime_ms;	// PID update time in milliseconds
    unsigned long m_lastupdatetime;
};

#endif //PID2_H

// Local Variables:
// c-basic-offset: 4
// End: 
