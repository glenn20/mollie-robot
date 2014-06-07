// -*- c++ -*-

#ifndef ROBOTMOTORS_H
#define ROBOTMOTORS_H

class EncoderParams {
public:
  EncoderParams( int   controlpin,
		 int   interruptnumber,
		 void  (*interruptfunction)() )
    : arduino_controlpin        ( controlpin ),
      arduino_interruptnumber   ( interruptnumber ),
      arduino_interruptfunction ( interruptfunction )
    {};

  int arduino_controlpin;
  int arduino_interruptnumber;
  void (*arduino_interruptfunction)();
};

static const unsigned int NPULSES = 9;

class Encoder {
public:
  Encoder( EncoderParams &params )
  : m_params ( params ),
    pulse    ( 1000 ),
    count    ( 0 ),
    lasttime ( 0 ),
    ndx      ( 0 ),
    npulses  ( 0 ),
    ntime    ( 0 ) 
  {};


  void initialise();

  // Record the next encoder pulse
  void update();

  // Return the encoder speed in pulses per second
  double speed();

  // Return the total number of encoder pulses recorded
  unsigned long totalCount();

private:
  EncoderParams&         m_params; 
  volatile unsigned long pulse;           // The width of the last pulse
  volatile unsigned long count;           // Total number of pulses recorded
  volatile unsigned long lasttime;        // The time of the last pulse
  volatile unsigned int  ndx;             // Where to put the next pulse time
  volatile unsigned int  npulses;         // Number of pulses recorded in array
  volatile unsigned long ntime;           // The time between the last npulses pulses
  volatile boolean       resetspeed;      // Flag to reset the speed variables
  volatile unsigned long times[NPULSES];  // The times of the last nine pulses
};

class MyPID {
public:
  MyPID()
    :  m_actualspeed( 0.0 ),
       m_setspeed( 0.0 ),
       m_output( 0.0 ),
       m_pid(&m_actualspeed, &m_output, &m_setspeed, 0.5, 0, 0, DIRECT) {
    m_pid.SetOutputLimits(-50, 50);
    m_pid.SetMode(AUTOMATIC);
    m_pid.SetSampleTime( 500 );
  };
  
  boolean UpdatePID( double setspeed, double measuredspeed, double* output ) {
    m_actualspeed = measuredspeed;
    m_setspeed = setspeed;
    if (m_pid.Compute()) {
      *output = m_output;
      return true;
    }
    return false;
  };
  
private:
  double m_actualspeed;
  double m_setspeed;
  double m_output;
  PID    m_pid;
};

class MotorParams {
public:
  MotorParams( int controlpin1, int controlpin2, int enablepin,
	       EncoderParams &encoderparams )
    : arduino_controlpin1( controlpin1 ),
      arduino_controlpin2( controlpin2 ),
      arduino_enablepin  ( enablepin ),
      encoder            ( encoderparams )
    {};
  int arduino_controlpin1;
  int arduino_controlpin2;
  int arduino_enablepin;
  EncoderParams& encoder;
};

class Motor {
public:
  Motor( MotorParams &params )
  : encoder          ( params.encoder ),
    pid              ( ),
    m_params         ( params ),
    m_motorDirection ( -1 ),
    m_motorSpeed     ( 0 ),
    m_motorEnabled   ( 1 ),
    m_pwm            ( 0 ) 
  {};

  void initialise();
  void run( int speed, int direction );
  void run( int speed );
  void setspeed( int speed );
  void setdirection( int direction );
  void enable();
  void disable();
  int  speed();
  void setPWM( int pwm );
  int  pwm();
  boolean Loop( String name );

  Encoder encoder;
  MyPID   pid;
private:
  MotorParams& m_params;
  int m_motorDirection;
  int m_motorSpeed;
  int m_motorEnabled;
  int m_pwm;
};

class HeadParams {
public:
  HeadParams( int servopin )
    : arduino_servopin( servopin )
    {};

  int arduino_servopin;
};

class Head {
public:
  Head( HeadParams &params )
    : m_params          ( params ),
      m_servo           ( ),
      m_horizontalangle ( 0.0 )
      {};

  void initialise();
  float look( float angle );     // Turn head to the given angle
  float angle();                 // Get the current head turn angle

private:
  HeadParams& m_params;
  Servo       m_servo;
  //MyPID     m_pid;
  float       m_horizontalangle;
};

class RobotParams {
public:
  RobotParams( MotorParams &leftwheel_,
	       MotorParams &rightwheel_,
	       HeadParams  &head_ )
    : leftwheel  ( leftwheel_ ),
      rightwheel ( rightwheel_ ),
      head       ( head_ )
    {}
      
  MotorParams &leftwheel;
  MotorParams &rightwheel;
  HeadParams  &head;
};

class Robot {
public:
  Robot( RobotParams& params )
    : leftwheel    ( params.leftwheel ),
      rightwheel   ( params.rightwheel ),
      head         ( params.head ),
      m_headangle  ( 0 ),
      m_leftspeed  ( 0 ),
      m_rightspeed ( 0 ),
      m_speed      ( 0 ),
      m_direction  ( 0 ) 
    {};
  
  boolean Go( int speed, int direction );
  float   look( float angle );
    
  void    initialise();
  void    enable();
  void    disable();
  int     leftspeed();
  int     rightspeed();
  int     speed();
  int     direction();

  boolean Loop();

  Motor   leftwheel;
  Motor   rightwheel;
  Head    head;
private:
  float   m_headangle;
  int     m_leftspeed;
  int     m_rightspeed;
  int     m_speed;
  int     m_direction;
};


#endif ROBOTMOTORS_H

// Local Variables:
// c-basic-offset: 2
// End: 
