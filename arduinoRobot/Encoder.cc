// -*- c++ -*-

#include <Arduino.h>

#include "Encoder.h"

Encoder::Encoder( int   controlpin,
		  int   interruptnumber,
		  void  (*interruptfunction)()
    )
    : m_controlpin        ( controlpin ),
      m_interruptnumber   ( interruptnumber ),
      m_interruptfunction ( interruptfunction ),
      m_count             ( 0 ),
      m_lasttime          ( 0 ),
      m_ndx               ( 0 ),
      m_npulses           ( 0 ),
      m_ntime             ( 0 )
{
}

void Encoder::initialise()
{
    if (m_controlpin >= 0) {
	// Set pullup resistors for Encoder
	digitalWrite( m_controlpin, 1 );
    
	// Set the encoder interrupts to call the interruptfunction
	attachInterrupt( m_interruptnumber,
			 m_interruptfunction,
			 CHANGE );
	Serial.print( "Encoder::initialise: controlpin=" );
	Serial.print( m_controlpin );
	Serial.print( " interrupt=" );
	Serial.println( m_interruptnumber );
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
    m_lasttime = t;
    m_count++;
    m_times[m_ndx] = t;
    if (m_npulses < NPULSES) {
	m_npulses++;
    }
    if (m_npulses >= 2) {
	// Difference between this reading and the oldest reading in the buffer
	m_ntime = m_times[m_ndx] - m_times[(m_ndx + 1) % m_npulses];
    } else {
	m_ntime = 0;
    }
    m_ndx = (m_ndx + 1) % NPULSES;
}

// Return the speed in pulses per second
double Encoder::speed()
{
    // No pulses have been recorded - just return a speed of zero
    if (m_ntime == 0) {
	return 0.0;
    }
    // If more than 0.1 seconds since last pulse, return a speed of zero
    if (micros() - m_lasttime > 100000) {
	return 0.0;
    }

    // Speed is the (number of pulses -1) / (time between first and last pulse)
    double ntime_seconds = m_ntime / 1000000.0;
    return (m_npulses - 1) / ntime_seconds;
}

unsigned long Encoder::count()
{
    return m_count;
}

// Local Variables:
// c-basic-offset: 4
// End: 
