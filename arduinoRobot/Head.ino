// -*- c++ -*-

#include "Head.h"

static const int servoMin =   0;   // looking left
static const int servoMax = 179;   // looking right

static const int angleMin = -90;
static const int angleMax =  90;

void Head::initialise() {
  m_servo.attach( m_servopin );
  Serial.print( "Head::initialise: servopin=" );
  Serial.println( m_servopin );

  look( 0.0 );
}

float Head::look( float angle ) {
  m_horizontalangle = constrain( angle, angleMin, angleMax );

  // Servo expects angle between 0 and 179
  int servoangle = constrain( angle + 90, servoMin, servoMax );
  m_servo.write( servoangle );
    
  return m_horizontalangle;
}

float Head::angle() {
  return m_horizontalangle;
}

// Local Variables:
// c-basic-offset: 2
// End: 
