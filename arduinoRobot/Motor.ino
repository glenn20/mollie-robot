// -*- c++ -*-

#include "Motor.h"

void Motor::initialise()
{
  // Initialise the arduino pins to control the DC motor
  pinMode( m_controlpin1, OUTPUT );
  pinMode( m_controlpin2, OUTPUT );
  pinMode( m_enablepin,   OUTPUT );
  disable();
  
  Serial.print( "Motor::initialise: Pins = ");
  Serial.print( m_controlpin1 );
  Serial.print( " " );
  Serial.print( m_controlpin2 );
  Serial.print( " " );
  Serial.print( m_enablepin );
  Serial.println( " " );
  
  encoder.initialise();
  
  //digitalWrite(m_enablePin, LOW);
}

void Motor::setPWM( int pwm )
{
  m_pwm = pwm;
  if(m_motorEnabled == 1){
    analogWrite(m_enablepin, m_pwm);
    //Serial.print( "Motor::setPWM: pin=");
    //Serial.print( m_enablepin );
    //Serial.print( " pwm=" );
    //Serial.println( m_pwm );
  }
//  else{
//    analogWrite(m_enablePin, 0);
//  }
}

void Motor::setdirection( int direction )
{
  if (m_motorDirection == direction) {
    return;
  }
  m_motorDirection = direction;
  digitalWrite(m_controlpin1, (direction == 1) ? HIGH : LOW );
  digitalWrite(m_controlpin2, (direction == 1) ? LOW  : HIGH);
  delay(1);
}

void Motor::enable()
{
  m_motorEnabled = 1;
  // digitalWrite(m_enablePin, HIGH);
  analogWrite(m_enablepin, m_pwm);
}

void Motor::disable()
{
  m_motorEnabled = 0;
  digitalWrite(m_enablepin, LOW);
}

void Motor::setspeed( int speed )
{
  m_motorSpeed = speed;
  //setPWM( m_motorSpeed );
}

int Motor::speed()
{
  return m_motorSpeed;
}

void Motor::run( int speed, int direction )
{
  setdirection( direction );
  setspeed( speed );
}

void Motor::run( int speed )
{
  setdirection( speed < 0 ? 0 : 1 );
  setspeed( speed < 0 ? -speed : speed );
}

int Motor::pwm()
{
  return m_pwm;
}

boolean Motor::Loop(String name)
{

  double pidoutput = 0.0;
  boolean updated = false;
  if (m_motorSpeed == 0) {
    if (m_pwm != 0) {
      updated= true;
      setPWM( 0 );
    }
  } else if (pid.UpdatePID( m_motorSpeed, encoder.speed()*10.0, &pidoutput )) {
    updated = true;
    const int pwm_min = 100;
    if (m_pwm < pwm_min) {
      m_pwm = pwm_min;
    }
    double newpwm = constrain( m_pwm + pidoutput, 100, 255 );
    setPWM( newpwm );
  }
  
  static int tick = -1;
  tick++;
  if (tick % 10000 == 0 || tick % 10000 == 1) {
    //Serial.print(name + ": ");
    //Serial.print("setspeed = ");
    //Serial.print(m_motorSpeed);
    //Serial.print(" actual = ");
    //Serial.print(encoder.speed()*10.0);
    //Serial.print(" pidoutput = ");
    //Serial.print(pidoutput);
    //Serial.print(" pwm = ");
    //Serial.print(m_pwm);
    //Serial.print(" count = ");
    //Serial.print(encoder.totalCount());
    //Serial.println("");
  }

  return updated;
}

// Local Variables:
// c-basic-offset: 2
// End: 
