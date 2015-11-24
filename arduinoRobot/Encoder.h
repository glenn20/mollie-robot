// -*- c++ -*-

#ifndef ENCODER_H
#define ENCODER_H

// An "Encoder" counts pulses and calculates wheel speed
class Encoder {
public:
    Encoder( void (*interruptfunction)() );

    void     initialise( int controlpin );
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
    volatile float         m_speed;
    volatile unsigned long m_count;          // Total number of pulses recorded
    volatile unsigned long m_lastcount;      // Time between last npulses
    volatile unsigned long m_lasttime;       // The time of the last pulse
};

#endif // ENCODER_H

// Local Variables:
// c-basic-offset: 4
// End: 
