// -*- c++ -*-

#include <Arduino.h>

#include "Encoder.h"

static const unsigned int NPULSES = 8;
static const unsigned int NMICROS = 50000;

Encoder::Encoder( void (*interruptfunction)() )
    : m_controlpin        ( 0 ),
      m_interruptfunction ( interruptfunction ),
      m_speed             ( 0.0 ),
      m_count             ( 0 ),
      m_lastcount	  ( 0 ),
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
    m_speed = 0.0;
    m_count = m_lastcount = m_lasttime = 0;

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

// The encoder generates 4 pulses per revolution of the wheel. The spacing
// between pulses is not perfectly uniform, so calculating the speed from the
// time between the last two pulses is inaccurate and noisy.

// Function to call from the arduino interrupt to register a pulse
void Encoder::update()
{
    unsigned long t = micros();
    ++m_count;
    if (m_lasttime == 0) {
	m_lastcount = m_count;
	m_lasttime  = t;
    } else if (t - m_lasttime > NMICROS) {
	m_speed     = ((m_count - m_lastcount) / ((t - m_lasttime) / 1.0e6));
	m_lastcount = m_count;
	m_lasttime  = t;
    }
}

// Return the speed in pulses per second
float Encoder::speed()
{
    unsigned long lasttime = m_lasttime;
    unsigned long t = micros();
    if (!valid() || lasttime == 0) {
	// Wheel is at rest - or no encoder present
	return 0.0;
    }
    if (t > lasttime && (t - lasttime) > 200000) {
	// If more than 0.5 seconds since last pulse, set the Wheel to be at rest
	m_lasttime = 0;
	Serial.print( "Wheel is stopping: " );
	Serial.print( t );
	Serial.print( " " );
	Serial.println( lasttime );
	return 0.0;
    }

    return m_speed / 24.0;
}

bool Encoder::moving()
{
    unsigned long lasttime = m_lasttime;
    unsigned long t = micros();
    return (lasttime != 0) && t < lasttime && (t - lasttime) < 200000;
}

unsigned long Encoder::count()
{
    return m_count;
}

// Local Variables:
// c-basic-offset: 4
// End: 
