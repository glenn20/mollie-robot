// -*- c++ -*-

#include <Arduino.h>

#include "Encoder.h"

Encoder::Encoder( int   controlpin,
		  void  (*interruptfunction)()
    )
    : m_controlpin        ( controlpin ),
      m_interruptfunction ( interruptfunction ),
      m_count             ( 0 ),
      m_lasttime          ( 0 ),
      m_ndx               ( 0 ),
      m_npulses           ( 0 ),
      m_ntime             ( 0 ),
      m_speed		  ( 0.0 ),
      m_reset		  ( false )
{
}

void Encoder::initialise()
{
    if (m_controlpin >= 0) {
	// Set pullup resistors for Encoder
	digitalWrite( m_controlpin, 1 );
    
	// Set the encoder interrupts to call the interruptfunction
	attachInterrupt( digitalPinToInterrupt( m_controlpin ),
			 m_interruptfunction,
			 CHANGE );
	Serial.print( "Encoder::initialise: controlpin=" );
	Serial.print( m_controlpin );
	Serial.print( " interrupt=" );
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
    unsigned long t = micros();
    // If more than 0.5 seconds since last pulse, reset the counters
    if (m_reset) {
	m_reset     = false;
	m_ntime     = 0;
	m_ndx       = 0;
	m_npulses   = 0;
	m_speed     = 0.0;
    }
    m_lasttime = t;
    m_count++;
    m_times[m_ndx] = t;
    if (m_npulses < NPULSES) {
	m_npulses++;
    }
    if (m_npulses >= 2) {
	// Difference between this reading and the oldest reading in the buffer
	m_ntime = m_times[m_ndx] - m_times[(m_ndx + 1) % m_npulses];
	m_speed = (1000000.0 * (m_npulses - 1)) / m_ntime;
    } else {
	m_ntime = 0;
	m_speed = 0.0;
    }
    m_ndx = (m_ndx + 1) % NPULSES;
}

// Return the speed in pulses per second
float Encoder::speed()
{
    if (!valid()) {
	return 0.0;
    }
    // If more than 0.5 seconds since last pulse, reset the counters
    if ((micros() - m_lasttime) > 500000 && m_ntime != 0) {
	m_reset = true;
	return 0.0;
    }

    return m_speed;
}

unsigned long Encoder::count()
{
    return m_count;
}

// Local Variables:
// c-basic-offset: 4
// End: 
