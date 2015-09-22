// -*- c++ -*-

#ifndef PID2_H
#define PID2_H

class MyPID2 {
public:
    MyPID2(
	float scale	 = 0.0,		// Scale factor to apply
	float Kp         = 0.0,		// PID proportionality constant
	float Kd         = 0.0,		// PID derivative constant
	float updatetime = 0.1		// Update period - in seconds
	) : m_scale     ( scale ),
	    m_Kp        ( Kp ),
	    m_Kd        ( Kd ),
	    m_lasterror ( 0 ),
	    m_updatetime( updatetime * 1000 ),  // Convert to milliseconds
	    m_lastupdatetime( 0 )
	{
	};

    bool UpdatePID(
	float&  correction,
	float   targetValue,
	float   currentValue
	)
	{
	    int t = millis();
	    if (t - m_lastupdatetime >= m_updatetime) {
		m_lastupdatetime = t;
		float error      = targetValue - currentValue; 
		correction       = m_scale * ((m_Kp * error) +
					      (m_Kd * (error - m_lasterror)));
		m_lasterror      = error;
		return true;
	    }
	    return false;
	};

    void setKp( float Kp ) { m_Kp = Kp; };
    void setKd( float Kd ) { m_Kd = Kd; };
    void setK( float Kp, float Kd ) { setKp( Kp ); setKd( Kd ); };

    float Kp() { return m_Kp; };
    float Kd() { return m_Kd; };

    void reset() { m_lasterror = 0.0; };

private:
    float    m_scale;
    float    m_Kp;			// PID proportionality constant
    float    m_Kd;			// PID derivative constant
    float    m_lasterror;		// Save the last error - for next PID
    float    m_updatetime;		// PID update time in milliseconds
    int      m_lastupdatetime;
};

#endif //PID2_H

// Local Variables:
// c-basic-offset: 4
// End: 
