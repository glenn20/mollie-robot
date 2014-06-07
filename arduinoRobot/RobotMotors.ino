// -*- c++ -*-

#include "RobotMotors.h"

// Record next encoder pulse
void Encoder::initialise()
{
  // Set pullup resistors for Encoder
  digitalWrite(m_params.arduino_controlpin, 1);
  
  // Set the encoder interrupts to call the interruptfunction
  attachInterrupt( m_params.arduino_interruptnumber,
		   m_params.arduino_interruptfunction,
		   CHANGE);
  Serial.print("Encoder::initialise: controlpin=");
  Serial.print(m_params.arduino_controlpin);
  Serial.print(" interrupt=");
  Serial.println(m_params.arduino_interruptnumber);
}

// The encoder generates 8 pulses per revolution of the wheel. The spacing
// between pulses is not perfectly uniform, so calculating the speed from the
// time between the last two pulses is inaccurate and noisy.

// We save the times (in microseconds) of the last 9 pulses to calculate an
// average speed for the last full wheel revolution. This leads to a much
// stabler speed measurement, but may lag behind the instantaneous speed when
// the speed is changing rapidly at low speed.

// npulses and ntime is used to calculate the speed in Encoder::speed().

// npulses is normally 9, but may be less if we have not yet recorded a full 9
// pulses

// ntime is the time (in microseconds) between the 9th and last most recent
// pulses

// Function to call from the arduino interrupt to register a pulse
void Encoder::update()
{
  unsigned long t = micros();
  pulse = t - lasttime;
  lasttime = t;
  count++;
  times[ndx] = lasttime;
  if (npulses < NPULSES) {
    npulses++;
  }
  if (npulses >= 2) {
    ntime = times[ndx] - times[(ndx+1)%npulses];
  } else {
    ntime = 0;
  }
  ndx = (ndx+1)%NPULSES;
}

// Return the speed in pulses per second
double Encoder::speed()
{  
  // No pulses have been recorded - just return a speed of zero
  if (ntime == 0) {
    return 0.0;
  }
  // If more than 0.8 seconds since last pulse received, return a speed of zero
  if (micros() - lasttime > 800000) {
    return 0.0;
  }

  // Speed is the (number of pulses -1) / (time between first and last pulse)
  double ntime_seconds = ntime / 1000000.0;
  return (npulses-1) / ntime_seconds;
}

unsigned long Encoder::totalCount()
{
  return count;
}

void Motor::initialise()
{
  // Initialise the arduino pins to control the DC motor
  pinMode( m_params.arduino_controlpin1, OUTPUT );
  pinMode( m_params.arduino_controlpin2, OUTPUT );
  pinMode( m_params.arduino_enablepin,   OUTPUT );
  disable();
  
  Serial.print( "Motor::initialise: Pins = ");
  Serial.print( m_params.arduino_controlpin1 );
  Serial.print( " " );
  Serial.print( m_params.arduino_controlpin2 );
  Serial.print( " " );
  Serial.print( m_params.arduino_enablepin );
  Serial.println( " " );
  
  encoder.initialise();
  
  //digitalWrite(m_enablePin, LOW);
}

void Motor::setPWM( int pwm )
{
  m_pwm = pwm;
  if(m_motorEnabled == 1){
    analogWrite(m_params.arduino_enablepin, m_pwm);
    //Serial.print( "Motor::setPWM: pin=");
    //Serial.print( m_params.arduino_enablepin );
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
  if(m_motorDirection == 1){
    digitalWrite(m_params.arduino_controlpin1, HIGH);
    digitalWrite(m_params.arduino_controlpin2, LOW);
  } else{
    digitalWrite(m_params.arduino_controlpin1, LOW);
    digitalWrite(m_params.arduino_controlpin2, HIGH);
  }
  delay(1);
}

void Motor::enable()
{
  m_motorEnabled = 1;
  // digitalWrite(m_enablePin, HIGH);
  analogWrite(m_params.arduino_enablepin, m_pwm);
}

void Motor::disable()
{
  m_motorEnabled = 0;
  digitalWrite(m_params.arduino_enablepin, LOW);
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

static const int servoMin =   0;   // looking left
static const int servoMax = 179;   // looking right

static const int angleMin = -90;
static const int angleMax =  90;

void Head::initialise() {
  m_servo.attach( m_params.arduino_servopin );
  Serial.print( "Head::initialise: servopin=");
  Serial.println( m_params.arduino_servopin );

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
  }
  else if (left < -255) {
    left  = -255;
    right = left + difference;
  }
  if (right > 255) {
    right = 255;
    left  = right + difference;
  }
  else if (right < -255) {
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
