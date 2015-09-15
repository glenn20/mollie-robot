// -*- c++ -*-

#ifndef ENCODER_H
#define ENCODER_H

static const unsigned int NPULSES = 9;

// An "Encoder" counts pulses and calculates wheel speed
class Encoder {
public:
    Encoder( int   controlpin,
	     int   interruptnumber,
	     void  (*interruptfunction)()
	);
  
    void     initialise();
    bool     valid();      // Return true if this a valid encoder
    void     update();     // Interrupt routine - record the encoder count
    double   speed();      // Return the encoder speed in pulses per second

    // Return the total number of encoder pulses recorded
    unsigned long count();

private:
    int                    m_controlpin;
    int                    m_interruptnumber;
    void                   (*m_interruptfunction)();
    volatile unsigned long m_count;          // Total number of pulses recorded
    volatile unsigned long m_lasttime;       // The time of the last pulse
    volatile unsigned int  m_ndx;            // Where to put next pulse time
    volatile unsigned int  m_npulses;        // # of pulses recorded in array
    volatile unsigned long m_ntime;          // Time between last npulses
    volatile bool          m_resetspeed;     // Flag to reset speed variables
    volatile unsigned long m_times[NPULSES]; // Times of the last nine pulses
};

#endif // ENCODER_H

// Local Variables:
// c-basic-offset: 4
// End: 
