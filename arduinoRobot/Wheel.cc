// -*- c++ -*-

#include "Wheel.h"

Wheel::Wheel( Motor&      motor,
	      Encoder&    encoder,
	      String      name
  )
  :           m_motor     ( motor ),
	      m_encoder   ( encoder ),
	      m_name      ( name ),
	      pid         ( ),
	      m_speed     ( 0 )
{
}

void Wheel::initialise()
{
  m_motor.initialise();
  m_encoder.initialise();
}

void Wheel::enable()
{
  m_motor.enable();
}

void Wheel::disable()
{
  m_motor.disable();
}

int Wheel::setspeed( int speed )
{
  m_speed = speed;
  if (!m_encoder.valid()) {
    // If we have encoder - set the power directly - skip the PID
    m_speed = m_motor.setpower( speed );
    // Loop();
  }
  return m_speed;
}

int Wheel::setspeed()
{
  return m_speed;
}

int Wheel::speed()
{
  return (m_encoder.valid()) ? m_encoder.speed() : m_speed;
}

int Wheel::power()
{
  return m_motor.power();
}

bool Wheel::Loop()
{
  if (!m_encoder.valid()) {
    return false;
  }
  double  pidoutput = 0.0;
  boolean updated   = false;
  int     power     = m_motor.power();
  float   speed     = m_encoder.speed();
  if (m_speed == 0) {
    if (power != 0) {
      updated = true;
      m_motor.setpower( 0 );
    }
  } else if (pid.UpdatePID( m_speed, speed*10.0, &pidoutput )) {
    updated = true;
    const int power_min = 100;
    if (power < power_min) {
      power = power_min;
    }
    double newpower = constrain( power + pidoutput, 100, 255 );
    m_motor.setpower( newpower );
  }
  
  static int tick = -1;
  tick++;
  if (tick % 2000 == 0 || tick % 2000 == 1) {
    Serial.print(m_name + ": ");
    Serial.print("setspeed = ");
    Serial.print(m_speed);
    Serial.print(" actual = ");
    Serial.print(speed);
    Serial.print(" pidoutput = ");
    Serial.print(pidoutput);
    Serial.print(" power = ");
    Serial.print(m_motor.power());
    Serial.print(" count = ");
    Serial.print(m_encoder.totalCount());
    Serial.println("");
  }

  return updated;
}

// Local Variables:
// c-basic-offset: 2
// End: 
