// -*- c++ -*-

#include "Robot.h"

Robot::Robot( Wheel&       leftwheel,
	      Wheel&       rightwheel,
	      Head&        head
    )
    :         m_leftwheel  ( leftwheel ),
	      m_rightwheel ( rightwheel ),
	      m_head       ( head ),
	      m_leftspeed  ( 0 ),
	      m_rightspeed ( 0 ),
	      m_speed      ( 0 ),
	      m_direction  ( 0 ) 
{
}

void Robot::initialise()
{
    m_leftwheel.initialise();
    m_rightwheel.initialise();
    m_head.initialise();
}

void Robot::enable()
{
    m_leftwheel.enable();
    m_rightwheel.enable();
}

void Robot::disable()
{
    m_leftwheel.disable();
    m_rightwheel.disable();
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

bool Robot::run( int speed, int direction )
{
    if (speed < -255) {
	speed = -255;
    } else if (speed > 255) {
	speed = 255;
    }
    if (direction < -1000) {
	direction = -1000;
    } else if (direction > 1000) {
	direction = 1000;
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

    m_leftwheel.run( left );
    m_rightwheel.run( right );

    return true;
}

void Robot::look( float angleX, float angleY )
{
    m_head.look( angleX, angleY );
}

void Robot::look( float angleX )
{
    m_head.look( angleX );
}

bool Robot::Loop()
{
    m_leftwheel.Loop();
    m_rightwheel.Loop();

    return true;
}

// Local Variables:
// c-basic-offset: 4
// End: 
