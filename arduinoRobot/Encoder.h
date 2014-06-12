// -*- c++ -*-

#ifndef ENCODER_H
#define ENCODER_H

static const unsigned int NPULSES = 9;

class Encoder {
public:
  Encoder( int   controlpin,
	   int   interruptnumber,
	   void  (*interruptfunction)()
    )
    : arduino_controlpin        ( controlpin ),
      arduino_interruptnumber   ( interruptnumber ),
      arduino_interruptfunction ( interruptfunction ),
      pulse    ( 1000 ),
      count    ( 0 ),
      lasttime ( 0 ),
      ndx      ( 0 ),
      npulses  ( 0 ),
      ntime    ( 0 ) 
    {};

  void     initialise(); // setup the encoder
  void     update();     // Interrrupt routine - record the encoder count
  double   speed();      // Return the encoder speed in pulses per second

  // Return the total number of encoder pulses recorded
  unsigned long totalCount();

private:
  int                    arduino_controlpin;
  int                    arduino_interruptnumber;
  void                   (*arduino_interruptfunction)();
  volatile unsigned long pulse;           // The width of the last pulse
  volatile unsigned long count;           // Total number of pulses recorded
  volatile unsigned long lasttime;        // The time of the last pulse
  volatile unsigned int  ndx;             // Where to put the next pulse time
  volatile unsigned int  npulses;         // Number of pulses recorded in array
  volatile unsigned long ntime;           // The time between the last npulses pulses
  volatile boolean       resetspeed;      // Flag to reset the speed variables
  volatile unsigned long times[NPULSES];  // The times of the last nine pulses
};

#endif ENCODER_H

// Local Variables:
// c-basic-offset: 2
// End: 
