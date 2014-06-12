// -*- c++ -*-

#ifndef HEAD_H
#define HEAD_H

class Head {
public:
  Head(    int servopin
    )
  :        m_servopin        ( servopin ),
	   m_servo           ( ),
	   m_horizontalangle ( 0.0 )
    {};

  void     initialise();
  float    look( float angle );     // Turn head to the given angle
  float    angle();                 // Get the current head turn angle

private:
  int      m_servopin;
  Servo    m_servo;
  //MyPID  m_pid;
  float    m_horizontalangle;
};

#endif HEAD_H

// Local Variables:
// c-basic-offset: 2
// End: 
