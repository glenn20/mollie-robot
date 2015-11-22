// -*- c++ -*-

#include "Head.h"

#include <Arduino.h>

HeadServo::HeadServo()
    : m_servo   ( ),
      m_angle   ( 0.0 ),
      m_servopin( 0 ),
      m_min     ( 0 ),
      m_origin  ( 0 ),
      m_max     ( 0 )
{
}

void HeadServo::initialise(
    int    servopin,
    int    min,
    int    origin,
    int    max 
    )
{
    if (m_servopin > 0) {
	close();
    }
    m_servopin = servopin;
    m_min      = min;
    m_origin   = origin;
    m_max      = max;

    if (m_servopin > 0) {
	m_servo.attach( m_servopin );
    }
    setangle( 0.0 );

    Serial.print( F("MyServo::initialise: servopin=") );
    Serial.println( m_servopin );
}

void HeadServo::close()
{
    setangle( 0.0 );
    if (m_servopin > 0) {
	m_servo.detach();
    }
}

static int round_int( float r ) {
    return (r > 0.0) ? (r + 0.5) : (r - 0.5); 
}

float HeadServo::setangle( float angle )
{
    int servoangle = round_int( angle + m_origin );
    servoangle = constrain( servoangle, m_min, m_max );
    m_angle = servoangle - m_origin;

    if (m_servopin > 0) {
	m_servo.write( servoangle );
    }
  
    return m_angle;
}

float HeadServo::angle()
{
    return m_angle;
}

Head::Head(
    HeadServo&  servox,
    HeadServo&  servoy
    ) :
    m_servoX    ( servox ),
    m_servoY    ( servoy )
{
}

void Head::close()
{
    m_servoX.close();
    m_servoY.close();
}

float Head::lookX( float angle )
{
    return m_servoX.setangle( angle );
}

float Head::lookY( float angle )
{
    return m_servoY.setangle( angle );
}

void Head::look( float angleX, float angleY )
{
    lookX( angleX );
    lookY( angleY );
}

float Head::angleX()
{
    return m_servoX.angle();
}

float Head::angleY()
{
    return m_servoY.angle();
}

// Local Variables:
// c-basic-offset: 4
// End: 
