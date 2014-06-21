// -*- c++ -*-

#include "Head.h"

#include <Arduino.h>

static const int servoMin =   0;   // looking left
static const int servoMax = 179;   // looking right

static const int angleMin = -89;
static const int angleMax =  90;

MyServo::MyServo(
  int servopin
  ) : m_servopin( servopin ),
      m_angle   ( 0.0 ),
      m_servo   ( )
{
}

void MyServo::initialise()
{
  if (m_servopin > 0) {
    m_servo.attach( m_servopin );
  }
  Serial.print( "MyServo::initialise: servopin=" );
  Serial.println( m_servopin );

  setangle( 0.0 );
}

float MyServo::setangle( float angle )
{
  m_angle = constrain( angle, angleMin, angleMax );

  // Servo expects angle between 0 and 179
  int servoangle = constrain( angle + 90, servoMin, servoMax );
  if (m_servopin > 0) {
    m_servo.write( servoangle );
  }
    
  return m_angle;
}

float MyServo::angle()
{
  return m_angle;
}

Head::Head( int servoxpin, int servoypin )
  :  m_servoX ( servoxpin ),
     m_servoY ( servoypin )
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

float Head::angleX()
{
  return m_servoX.angle();
}

float Head::angleY()
{
  return m_servoY.angle();
}

void Head::look( float angleX, float angleY )
{
  lookX( angleX );
  lookY( angleY );
}

// Local Variables:
// c-basic-offset: 2
// End: 
