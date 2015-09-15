// -*- c++ -*-

#include "Wheel.h"

Wheel::Wheel(
    Motor&      motor,
    Encoder&    encoder,
    String      name,
    bool        usepid
    ) :
    m_motor     ( motor ),
    m_encoder   ( encoder ),
    m_name      ( name ),
    pid         ( ),
    m_speed     ( 0 ),
    m_usepid    ( true ),
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

int Wheel::setspeed( int speed )
{
    m_speed = speed;
    if (!m_encoder.valid() || !m_usepid) {
	// If we have encoder - set the power directly - skip the PID
	m_speed = m_motor.setpower( speed );
	// Loop();
    }
    return m_speed;
}

int Wheel::setspeed()
{
    return m_speed;
}

int Wheel::speed()
{
    return (m_encoder.valid()) ? m_encoder.speed() : m_speed;
}

int Wheel::power()
{
    return m_motor.power();
}

bool Wheel::Loop()
{
    if (!m_encoder.valid()) {
	return false;
    }
    // Increment our loop counter - see diagnostics below
    m_tick++;
    double  pidoutput = 0.0;
    boolean updated   = false;
    int     power     = m_motor.power();
    float   speed     = m_encoder.speed();
    if (m_speed == 0) {
	// If the requested speed is zero - just turn off the power
	if (power != 0) {
	    updated = true;
	    m_motor.setpower( m_speed );
	}
    } else if (m_usepid && pid.UpdatePID( m_speed, speed, &pidoutput )) {
	updated = true;
	const int power_min = 0;
	if (power < power_min) {
	    power = power_min;
	}
	double newpower = constrain( power + pidoutput, power_min, 250 );
	m_motor.setpower( newpower );
    }
    // if (m_tick > 2000) {
    //   m_tick = 0;
    //   Serial.print(m_name + ": ");
    //   Serial.print("setspeed = ");
    //   Serial.print(m_speed);
    //   Serial.print(" actual = ");
    //   Serial.print(speed);
    //   Serial.print(" pidoutput = ");
    //   Serial.print(pidoutput);
    //   Serial.print(" power = ");
    //   Serial.print(m_motor.power());
    //   Serial.print(" count = ");
    //   Serial.print(m_encoder.count());
    //   Serial.println("");
    // }

    return updated;
}

// Local Variables:
// c-basic-offset: 4
// End: 
