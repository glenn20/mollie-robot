// -*- c++ -*-

#include "Head.h"

#include <Arduino.h>

HeadServo::HeadServo(
    int    servopin,
    int    origin,
    int    min,
    int    max 
    ) : m_servopin( servopin ),
	m_angle   ( 0.0 ),
	m_servo   ( ),
	m_origin  ( origin ),
	m_min     ( min ),
	m_max     ( max )
{
}

void HeadServo::initialise()
{
    if (m_servopin > 0) {
	m_servo.attach( m_servopin );
    }
    setangle( 0.0 );

    Serial.print( "MyServo::initialise: servopin=" );
    Serial.println( m_servopin );
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
    Serial.print( "MyServo::setangle[pin=" );
    Serial.print( m_servopin );
    Serial.print( "] = " );
    Serial.println( servoangle );
  
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

void Head::initialise()
{
    m_servoX.initialise();
    m_servoY.initialise();
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
