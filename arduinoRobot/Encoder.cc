// -*- c++ -*-

#include <Arduino.h>

#include "Encoder.h"

Encoder::Encoder( void (*interruptfunction)() )
    : m_controlpin        ( 0 ),
      m_interruptfunction ( interruptfunction ),
      m_count             ( 0 ),
      m_deltat		  ( 0 ),
      m_lasttime          ( 0 )
{
}

void Encoder::initialise( int controlpin )
{
    // If already initialised, de-initialise...
    if (m_controlpin >= 0) {
	close();
    }

    m_controlpin = controlpin;
    m_count = m_deltat = m_lasttime = 0;

    if (m_controlpin >= 0) {
	pinMode( m_controlpin, INPUT_PULLUP );
	// digitalWrite( m_controlpin, 1 );   // Set pullup resistors for Encoder
	// Set the encoder interrupts to call the interruptfunction
	attachInterrupt( digitalPinToInterrupt( m_controlpin ),
			 m_interruptfunction,
			 RISING );
	Serial.print( F("Encoder::initialise: controlpin=") );
	Serial.print( m_controlpin );
	Serial.print( F(" interrupt=") );
	Serial.println( digitalPinToInterrupt( m_controlpin ) );
    }
}

void Encoder::close()
{
    if (m_controlpin >= 0) {
	detachInterrupt( digitalPinToInterrupt( m_controlpin ) );
    }
}

bool Encoder::valid()
{
    return (m_controlpin >= 0);
}

// The encoder generates 8 pulses per revolution of the wheel. The spacing
// between pulses is not perfectly uniform, so calculating the speed from the
// time between the last two pulses is inaccurate and noisy.

// We save the times (in microseconds) of the last 9 pulses to calculate an
// average speed for the last full wheel revolution. This leads to a much
// stabler speed measurement, but may lag behind the instantaneous speed when
// the speed is changing rapidly at low speed.

// npulses and ntime is used to calculate the speed in Encoder::speed().

// npulses is normally 9, but may be less if we have not yet recorded a full 9
// pulses

// ntime is the time (in microseconds) between the 9th and last most recent
// pulses

// Function to call from the arduino interrupt to register a pulse
void Encoder::update()
{
    if (++m_count % NPULSES != 0) {
	// Just keep counting pulses till we have enough to measure speed
	return;
    }
    unsigned long t = micros();
    if (m_lasttime != 0) {
	m_deltat = t - m_lasttime;
    }
    m_lasttime = t;
}

// Return the speed in pulses per second
float Encoder::speed()
{
    if (!valid() || m_lasttime == 0) {
	// Wheel is at rest - or no encoder present
	return 0.0;
    }
    if ((micros() - m_lasttime) > 200000) {
	// If more than 0.2 seconds since last pulse, set the Wheel to be at rest
	m_lasttime = 0;
	return 0.0;
    }

    unsigned long deltat = m_deltat;
    return (deltat > 0 ? (NPULSES / (deltat / 1000000.0)) / 24.0 : 0.0);
}

bool Encoder::moving()
{
    unsigned long lasttime = m_lasttime;
    return (lasttime != 0) && (micros() - lasttime) < 200000;
}

unsigned long Encoder::count()
{
    return m_count;
}

// Local Variables:
// c-basic-offset: 4
// End: 
