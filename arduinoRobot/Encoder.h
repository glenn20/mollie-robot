// -*- c++ -*-

#ifndef ENCODER_H
#define ENCODER_H

static const unsigned int NPULSES = 9;

// An "Encoder" counts pulses and calculates wheel speed
class Encoder {
public:
    Encoder( int   controlpin,
	     void  (*interruptfunction)()
	);
  
    void     initialise();
    void     close();
    bool     valid();      // Return true if this a valid encoder
    void     update();     // Interrupt routine - record the encoder count
    float    speed();      // Return the encoder speed in pulses per second
    bool     moving();     // Is the wheel moving?

    // Return the total number of encoder pulses recorded
    unsigned long count();

private:
    int                    m_controlpin;
    void                   (*m_interruptfunction)();
    volatile unsigned long m_count;          // Total number of pulses recorded
    volatile unsigned long m_lasttime;       // The time of the last pulse
    volatile unsigned int  m_ndx;            // Where to put next pulse time
    volatile unsigned int  m_npulses;        // # of pulses recorded in array
    volatile unsigned long m_ntime;          // Time between last npulses
    volatile float         m_speed;	     // The speed in counts per sec
    volatile bool          m_reset;
    volatile unsigned long m_times[NPULSES]; // Times of the last nine pulses
};

#endif // ENCODER_H

// Local Variables:
// c-basic-offset: 4
// End: 
