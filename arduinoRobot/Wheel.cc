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
    pid			( 0.2, 0.0, 0.0,
			  -255, 255, 200 ),
    m_setspeed		( 0.0 ),
    m_setspeedp		( false ),
    m_tick		( 0 ),
    m_startsequencep	( false ),
    m_started		( false ),
    m_startedtime	( 0 ),
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
    m_startsequencep = true
    return m_setspeed;
}

float Wheel::speed()
{
    return m_encoder.speed() * ((m_motor.power() < 0) ? -1.0 : 1.0);
}

int Wheel::setpower( int power )
{
    m_startsequencep = false;
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
    m_setspeedp      = false;
    m_startsequencep = false;
    m_motor.power();
}

bool Wheel::Loop()
{
    if (!m_setspeedp || !m_encoder.valid()) {
	return false;
    }
    int     mpower    = power();
    float   mspeed    = speed();
    // Encoder does not sense direction
    // - use the motor power to guess direction
    if (-0.001 < m_setspeed && m_setspeed < 0.001) {
	// If the requested speed is zero - just turn off the power and return
	if (mpower != 0) {
	    m_motor.setpower( 0 );
	    return true;
	}
	return false;
    }
    // If the actual speed is zero, start the startup sequence...
    unsigned long t = millis();
    unsigned long counts = m_encoder.count();
    bool stationary = (-0.001 < mspeed && mspeed < 0.001);
    if (stationary && !m_startsequencep) {
	// We are starting the wheels from rest - start the startup sequence
	m_startsequencep = true;
	m_starttime      = millis();
	m_startcount     = m_encoder.count();
	m_motor.setpower( 160 );
	Serial.print( "Startsequence=true: " );
	Serial.println( m_name );
    }
    if (m_startsequencep) {
	if (!(counts > m_startcount)) {
	    // First the high power - ramp up phase
	    // Keep ramping up power till the motors start moving 
	    m_motor.setpower( 160 + (t - m_starttime) * 0.01 );
	    return true;
	} else {
	    // Next, the low power phase
	    if (!m_started) {
		// Once the wheels start moving, reduce power
		m_started = true;
		m_startedtime = t;
		m_motor.setpower( mpower - 60 );
		return true;
	    }
	    // ...after that - do nothing until we register non-zero speed
	}
	if (!stationary) {
	    // The wheels are moving at speed - stop the start sequence
	    if (!m_started) {
		// If we haven't switched to low power mode - do that now
		m_started = true;
		m_startedtime = t;
		m_motor.setpower( mpower - 60 );
		return true;
	    }
	}
	if (m_started) {
	    // End the start sequence phase
	    if (t - m_startedtime > 10000) {
		// Allow time for speed to settle down before start PID
		m_startsequencep = false;
		Serial.print( "Startsequence=false: " );
		Serial.println( m_name );
		// Initialise the output power settings for the PID controller
		m_pidpower = power();
	    }
	    return false;
	}
    } else if (m_pidcontrolp) {
	// End of the start sequence...
	if (pid.UpdatePID( (double)m_setspeed, (double)mspeed, &m_pidpower )) {
	    double p = m_pidpower;
	    if (m_setspeed > 0.0) {
		p = ((p <    0) ?   0 :
		     ((p > 255) ? 255 :
		      p ));
	    } else {
		p = ((p >     0) ?    0 :
		     ((p < -255) ? -255 :
		      p ));
	    }
	    m_pidpower = p;
	    m_motor.setpower( (int) m_pidpower );
	    return true;
	}
    }

    return false;
}

// Local Variables:
// c-basic-offset: 4
// End: 
