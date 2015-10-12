// -*- c++ -*-

#include "Wheel.h"

Wheel::Wheel(
    Motor&      motor,
    Encoder&    encoder,
    String      name
    ) :
    m_motor		( motor ),
    m_encoder		( encoder ),
    m_name		( name ),
    pid			( 2.0, 0.1, 0.0, 0.2 ),
    m_setspeed		( 0.0 ),
    m_setspeedp		( false ),
    m_tick		( 0 ),
    m_startsequencep	( false ),
    m_started		( false ),
    m_starttime		( 0 ),
    m_startcount	( 0 )
{
}

void Wheel::initialise()
{
    m_motor.initialise();
    m_encoder.initialise();
}

void Wheel::enable()
{
    m_motor.enable();
}

void Wheel::disable()
{
    m_motor.disable();
}

float Wheel::setspeed( float speed )
{
    m_setspeed = speed;
    m_setspeedp = true;
    // Loop();

    return m_setspeed;
}

float Wheel::setspeed()
{
    return m_setspeed;
}

float Wheel::speed()
{
    return m_encoder.speed() * ((m_motor.power() < 0) ? -1.0 : 1.0);
}

int Wheel::setpower( int power )
{
    m_setspeedp = false;
    m_setspeed = 0.0;
    return m_motor.setpower( power );
}

float Wheel::power()
{
    return m_motor.power();
}

void Wheel::stop()
{
    m_setspeedp = false;
    m_motor.power();
}

bool Wheel::Loop()
{
    if (!m_encoder.valid()) {
	// No encoder means no speed control...
	return false;
    }
    if (!m_setspeedp) {
	return false;
    }
    // Increment our loop counter - see diagnostics below
    boolean updated   = false;
    int     fpower    = m_motor.power();
    float   fspeed    = speed();
    // Encoder does not sense direction
    // - use the motor power to guess direction
    float   correction;
    if (-0.001 < m_setspeed && m_setspeed < 0.001) {
	// If the requested speed is zero - just turn off the power
	if (fpower != 0) {
	    m_motor.setpower( 0 );
	    return true;
	}
	return false;
    }
    if (-0.001 < fspeed && fspeed < 0.001) {
	unsigned long t = millis();
	unsigned long counts = m_encoder.count();
	// We are starting the wheels from rest - start the startup sequence
	if (!m_startsequencep) {
	    m_startsequencep = true;
	    m_started        = false;
	    m_starttime      = t;
	    m_startcount     = counts;
	    // m_motor.setpower( 160 );
	}
	if (counts == m_startcount) {
	    // If the wheel hasn't started moving...
	    // Ramp up the power at 10 units per second... till the wheel moves
	    m_motor.setpower( 160 + (t - m_starttime) * 0.01 );
	} else {
	    if (!m_started) {
		// The wheel has started moving - reduce power
		m_started = true;
		m_motor.setpower( fpower - 80 );
	    }
	}
	return true;
    }
    // End of the start sequence...
    m_startsequencep = false;
    if (pid.UpdatePID( correction, m_setspeed, fspeed )) {
	updated = true;
	int p = fpower + correction;
	if (m_setspeed > 0.0) {
	    p = ((p <    0) ?   0 :
		 ((p > 255) ? 255 :
		  p ));
	} else {
	    p = ((p >     0) ?    0 :
		 ((p < -255) ? -255 :
		  p ));
	}
	m_motor.setpower( p );
    }

    return updated;
}

// Local Variables:
// c-basic-offset: 4
// End: 
