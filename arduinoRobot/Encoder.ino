// -*- c++ -*-

#include "Encoder.h"

// Record next encoder pulse
void Encoder::initialise()
{
  // Set pullup resistors for Encoder
  digitalWrite( arduino_controlpin, 1 );
  
  // Set the encoder interrupts to call the interruptfunction
  attachInterrupt( arduino_interruptnumber,
		   arduino_interruptfunction,
		   CHANGE );
  Serial.print("Encoder::initialise: controlpin=");
  Serial.print(arduino_controlpin);
  Serial.print(" interrupt=");
  Serial.println(arduino_interruptnumber);
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
  pulse = t - lasttime;
  lasttime = t;
  count++;
  times[ndx] = lasttime;
  if (npulses < NPULSES) {
    npulses++;
  }
  if (npulses >= 2) {
    ntime = times[ndx] - times[(ndx+1)%npulses];
  } else {
    ntime = 0;
  }
  ndx = (ndx+1)%NPULSES;
}

// Return the speed in pulses per second
double Encoder::speed()
{  
  // No pulses have been recorded - just return a speed of zero
  if (ntime == 0) {
    return 0.0;
  }
  // If more than 0.8 seconds since last pulse received, return a speed of zero
  if (micros() - lasttime > 800000) {
    return 0.0;
  }

  // Speed is the (number of pulses -1) / (time between first and last pulse)
  double ntime_seconds = ntime / 1000000.0;
  return (npulses-1) / ntime_seconds;
}

unsigned long Encoder::totalCount()
{
  return count;
}

// Local Variables:
// c-basic-offset: 2
// End: 
