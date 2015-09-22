// -*- c++ -*-

#include "Robot.h"

#include <ArduinoJson.h>


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
	      m_direction  ( 0 ),
	      m_tick       ( 0 )
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

int Robot::setpower( int power )
{
    m_leftwheel.setpower( power );
    m_rightwheel.setpower( power );

    return true;
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

    m_tick++;
    if (m_tick > 2000) {
	m_tick = 0;
	sendjson();
    }

    return true;
}

void Robot::sendjson()
{
    //Serial.println( "JSON" );
    //return;
    // The internal buffer for the Json objects
    StaticJsonBuffer<200> jsonBuffer;

    JsonObject& root = jsonBuffer.createObject();

    root["time"]     = millis();
    root["headX"]    .set( m_head.angleX() );
    root["headY"]    .set( m_head.angleY() );
    root["setspeedL"].set( m_leftwheel .setspeed() );
    root["setspeedR"].set( m_rightwheel.setspeed() );
    root["speedL"]   .set( m_leftwheel .speed() );
    root["speedR"]   .set( m_rightwheel.speed() );
    root["powerL"]   = m_leftwheel .power();
    root["powerR"]   = m_rightwheel.power();
    root["countsL"]  = m_leftwheel .encoder().count();
    root["countsR"]  = m_rightwheel.encoder().count();

    root.printTo( Serial );
    Serial.println();
}

// Local Variables:
// c-basic-offset: 4
// End: 
