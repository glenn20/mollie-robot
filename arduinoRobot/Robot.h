// -*- c++ -*-

#ifndef ROBOT_H
#define ROBOT_H

#include "Motor.h"
#include "Head.h"

class Robot {
public:
  Robot(   Motor&       leftwheel_,
	   Motor&       rightwheel_,
	   Head&        head_
    )
    :      leftwheel    ( leftwheel_ ),
	   rightwheel   ( rightwheel_ ),
	   head         ( head_ ),
	   m_headangle  ( 0 ),
	   m_leftspeed  ( 0 ),
	   m_rightspeed ( 0 ),
	   m_speed      ( 0 ),
	   m_direction  ( 0 ) 
    {};
  
  boolean  Go( int speed, int direction );
  float    look( float angle );
    
  void     initialise();
  void     enable();
  void     disable();
  int      leftspeed();
  int      rightspeed();
  int      speed();
  int      direction();

  boolean  Loop();

  Motor&   leftwheel;
  Motor&   rightwheel;
  Head&    head;
private:
  float    m_headangle;
  int      m_leftspeed;
  int      m_rightspeed;
  int      m_speed;
  int      m_direction;
};


#endif ROBOT_H

// Local Variables:
// c-basic-offset: 2
// End: 
