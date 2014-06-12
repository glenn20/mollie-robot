// -*- c++ -*-

#include "Robot.h"

void Robot::initialise()
{ 
  leftwheel.initialise(); 
  rightwheel.initialise();
  head.initialise();
}

void Robot::enable()
{
  leftwheel.enable();
  rightwheel.enable();
}

void Robot::disable()
{
  leftwheel.disable();
  rightwheel.disable();
};

int Robot::leftspeed()
{
  return m_leftspeed;
}

int Robot::rightspeed()
{
  return m_rightspeed;
}

int Robot::speed()
{
  return m_speed;
}

int Robot::direction()
{
  return m_direction;
}

boolean Robot::Go( int speed, int direction )
{
  if (speed < -255 || speed > 255) {
    return false;
  }
  if (direction < -1000 or direction > 1000) {
    return false;
  }
  int difference = direction * 500 / 1000;
  int left  = speed + 0.5 * difference;
  int right = speed - 0.5 * difference;
  if (left > 255) {
    left  = 255;
    right = left - difference;
  } else if (left < -255) {
    left  = -255;
    right = left + difference;
  }
  if (right > 255) {
    right = 255;
    left  = right + difference;
  } else if (right < -255) {
    right = -255;
    left  = right - difference;
  }
  m_speed = speed;
  m_direction = direction;
  m_leftspeed = left;
  m_rightspeed = right;

  leftwheel.run( left );
  rightwheel.run( right );

  return true;
}

float Robot::look( float angle )
{
  m_headangle = head.look( angle );
  return m_headangle;
}

boolean Robot::Loop()
{
  leftwheel.Loop ("Left ");
  rightwheel.Loop("Right");

  return true;
}

// Local Variables:
// c-basic-offset: 2
// End: 
