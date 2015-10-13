// -*- c++ -*-

#include "Wheel.h"

Wheel::Wheel(
    Motor&      motor,
    Encoder&    encoder,
    String      name
    ) :
    m_motor	   ( motor ),
    m_encoder	   ( encoder ),
    m_name	   ( name ),
    pid		   ( 0.2, 0.0, 0.0,
		     -255, 255, 200 ),
    m_tick	   ( 0 ),
    m_setspeed	   ( 0.0 ),
    m_controlstate ( PID_NONE ),
    m_starttime	   ( 0 ),
    m_startcount   ( 0 ),
    m_pidpower     ( 0 )
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
    if (-0.001 < speed && speed < 0.001) {
	// If the requested speed is zero - just turn off the power and return
	this->setpower( 0 );
	return 0.0;
    }
    if (m_controlstate == PID_NONE) {
	//Serial.print( "Controlstate0=" );
	//Serial.println( m_controlstate );
	m_controlstate = PID_STARTING;
	//Serial.print( "ControlstateA=" );
	//Serial.println( m_controlstate );
    }
    // Save the target speed - will be used by PID loop in Loop() below.
    m_setspeed = speed;

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
    if (m_controlstate != PID_NONE) {
	m_controlstate = PID_NONE;
	m_setspeed     = 0.0;
	//Serial.print( "ControlstateB=" );
	//Serial.println( m_controlstate );
    }
    return m_motor.setpower( power );
}

float Wheel::power()
{
    return m_motor.power();
}

void Wheel::stop()
{
    this->setpower( 0 );
}

bool Wheel::Loop()
{
    if (m_controlstate == PID_NONE || !m_encoder.valid()) {
	return false;
    }
    bool updated = false;
    double correction = 0.0;
    switch (m_controlstate) {
    case PID_NONE:
	break;
    case PID_STARTING:
	// We are starting the wheels from rest - start the startup sequence
	m_controlstate   = PID_HIGHPOWER;
	//Serial.print( "ControlstateC=" );
	//Serial.println( m_controlstate );
	m_starttime      = millis();
	m_startcount     = m_encoder.count();
	m_motor.setpower( 200 );
	updated = true;
	break;
    case PID_HIGHPOWER:
	if (!(m_encoder.count() > m_startcount)) {
	    // First the high power - ramp up phase
	    // Keep ramping up power till the motors start moving 
	    //m_motor.setpower( 160 + (millis() - m_starttime) * 0.01 );
	    //updated = true;
	} else {
	    // The wheels are moving at speed - stop the start sequence
	    m_controlstate = PID_LOWPOWER;
	    m_motor.setpower( 100 );
	    updated = true;
	    //Serial.print( "ControlstateD=" );
	    //Serial.println( m_controlstate );
	}
	break;
    case PID_LOWPOWER:
	// End the start sequence phase
	if (millis() - m_starttime > 10000) {
	    // Allow time for speed to settle down before start PID
	    m_controlstate = PID_PIDCONTROL;
	    // Initialise the output power settings for the PID controller
	    m_pidpower = this->power();
	    //Serial.print( "ControlstateE=" );
	    //Serial.println( m_controlstate );
	}
	break;
    case PID_PIDCONTROL:
	// Run PID control
	if (pid.UpdatePID( (double)m_setspeed,
			   (double)this->speed(),
			   &correction )) {
	    m_pidpower += correction;
	    // double p = m_pidpower;
	    // if (m_setspeed > 0.0) {
	    // 	p = ((p <    0) ?   0 :
	    // 	     ((p > 255) ? 255 :
	    // 	      p ));
	    // } else {
	    // 	p = ((p >     0) ?    0 :
	    // 	     ((p < -255) ? -255 :
	    // 	      p ));
	    // }
	    // m_pidpower = p;
	    m_motor.setpower( (int) m_pidpower );
	    updated = true;
	}
	break;
    default:
	break;
    }
 
    return updated;
}

// Local Variables:
// c-basic-offset: 4
// End: 
