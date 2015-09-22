// -*- c++ -*-

#include "Wheel.h"

Wheel::Wheel(
    Motor&      motor,
    Encoder&    encoder,
    String      name
    ) :
    m_motor     ( motor ),
    m_encoder   ( encoder ),
    m_name      ( name ),
    pid         ( 2.0, 0.8, 0.01, 0.5 ),
    m_setspeed  ( 0.0 ),
    m_setspeedp ( false ),
    m_tick      ( 0 )
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
    // Increment our loop counter - see diagnostics below
    boolean updated   = false;
    int     fpower    = power();
    float   fspeed    = speed();
    // Encoder does not sense direction - use the current motor power to guess direction
    bool    forwardp  = fspeed > 0.0;
    float   correction;
    if (m_setspeedp && -0.001 < m_setspeed && m_setspeed < 0.001) {
	// If the requested speed is zero - just turn off the power
	if (fpower != 0) {
	    updated = true;
	    m_motor.setpower( 0 );
	}
    } else if (m_setspeedp && pid.UpdatePID( correction, m_setspeed, fspeed )) {
	updated = true;
	m_motor.setpower( constrain( fpower + correction,
				     (forwardp ?   0 : -255),
				     (forwardp ? 255 :    0) ) );
    }

    return updated;
}

// Local Variables:
// c-basic-offset: 4
// End: 
