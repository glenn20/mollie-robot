// -*- c++ -*-

#include <PID_v1.h>

#include <string.h>

#include "Robot.h"
#include "Json.h"

Robot::Robot( Wheel&       leftwheel,
	      Wheel&       rightwheel,
	      Head&        head
    )
    :         m_leftwheel  ( leftwheel ),
	      m_rightwheel ( rightwheel ),
	      m_head       ( head ),
	      m_updated    ( false )
{
}

void Robot::initialise()
{
    m_leftwheel.initialise();
    m_rightwheel.initialise();
    m_head.initialise();
    m_updated = true;
}

void Robot::close()
{
    m_leftwheel.close();
    m_rightwheel.close();
    m_head.close();
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
    return m_leftwheel.speed();
}

int Robot::rightspeed()
{
    return m_rightwheel.speed();
}

int Robot::speed()
{
    return (leftspeed() + rightspeed()) / 2.0;
}

bool Robot::setpower( int left, int right )
{
    m_leftwheel.setpower( left );
    m_rightwheel.setpower( right );
    m_updated = true;

    return true;
}

bool Robot::run( int left, int right )
{
    m_leftwheel.run( left );
    m_rightwheel.run( right );
    m_updated = true;

    return true;
}

void Robot::look( float angleX, float angleY )
{
    m_head.look( angleX, angleY );
    m_updated = true;
}

void Robot::look( float angleX )
{
    m_head.look( angleX );
    m_updated = true;
}

bool Robot::Loop()
{
    if (m_leftwheel.Loop()) {
	m_updated = true;
    }
    if (m_rightwheel.Loop()) {
	m_updated = true;
    }

    if (m_updated) {
	m_json.update( "speed",
		       m_leftwheel .speed(),
		       m_rightwheel.speed() );
	m_json.update( "counts",
		       m_leftwheel .count(),
		       m_rightwheel.count() );
	//sendjson();
	m_updated = false;
    }
    if (m_json.send())
	Serial.flush();

    return true;
}

// --- Init PID Controller ---
double posX = 0.0;
double posY = 0.0;

//Define Variables we'll be connecting to
double SetpointX = 0, InputX = 0, OutputX = 0;
double SetpointY = 0, InputY = 0, OutputY = 0;
double Setpointbody = 0, Inputbody = 0, Outputbody = 0;

//Specify the links and initial tuning parameters
// face tracking: 0.8, 0.6, 0
// color tracking: 0.4, 0.4, 0
PID myPIDX(&InputX, &OutputX, &SetpointX, 0.2, 0.0, 0.0, DIRECT);
PID myPIDY(&InputY, &OutputY, &SetpointY, 0.2, 0.0, 0.0, DIRECT);
PID myPIDbody(&Inputbody, &Outputbody, &Setpointbody, 5.0, 0.0, 0.0, DIRECT);

void SetupPID()
{
    // --- Setup PID ---
    myPIDX.SetOutputLimits(-90, 90);
    myPIDY.SetOutputLimits(-90, 90);
    myPIDbody.SetOutputLimits(-100, 100);
    //turn PIDs on
    myPIDX.SetMode(AUTOMATIC);
    myPIDY.SetMode(AUTOMATIC);
    myPIDbody.SetMode(AUTOMATIC);
}

void Robot::dotrackingPID( int x, int y )
{
    static bool initialised = false;

    if (!initialised) {
	SetupPID();
	initialised = true;
    }
    // set servo angle
    // servo 0-180
    InputX = x;
    InputY = y;
    myPIDX.Compute();
    myPIDY.Compute();
    if (-2.0 < InputX && InputX < 2.0) {
	// Accept small angles
	OutputX = 0;
    }
    if (-2.0 < InputY && InputY < 2.0) {
	// Accept small angles
	OutputY = 0;
    }
    // Update Servo Position
    //posX = robbie.head().angleX();
    //posY = robbie.head().angleY();
    posX = constrain(posX + OutputX, -90, 90);
    posY = constrain(posY - OutputY, -90, 90);
    //Serial.print( "Camera direction: " ); Serial.println( posX );
    look( posX, posY );
    
    // Now, turn the body toward where the camera is looking
    //Inputbody = posX;
    //myPIDbody.Compute();

    //if (-5.0 < Outputbody && Outputbody < 5.0) {
    //    return true;
    //}
    //int direction = constrain( Outputbody, -100, 100 );
    //Serial.print( "Robot direction: " ); Serial.println( direction );
    //robbie.run( robbie.speed(), direction );
}

// Process a line of text containing commands to move the motors
// Returns true if any action was taken, else return false.
bool Robot::robotcommand( char* line )
{
    char command[20] = "";// A string to hold the command
    int  numbers[8];      // The list of numbers following the command

    if (line[0] == '{') {
	bool updated = processjson( line );
	return updated;
    }

    // Every command starts with the <command>, then up to eight integer numbers
    int n = sscanf( line, "%20s %d %d %d %d %d %d %d %d", 
		    command, 
		    &numbers[0], &numbers[1], &numbers[2], &numbers[3], 
		    &numbers[4], &numbers[5], &numbers[6], &numbers[7] );
    String cmd = command;
    n--;
    if (cmd == "track" && n == 2) {
	// int anglex = numbers[0];
	// int angley = numbers[1];
	// look( anglex, angley );
	dotrackingPID( numbers[0], numbers[1] );
    } else {
	Serial.print( "Unknown robot command: " ); Serial.println( line );
    }
    return true;
}

void Robot::sendjson()
{
    // Hand crafted pseudo-json - cause the json library seems to crash
    m_json.send();
    m_json.update( "head",
		   m_head.angleX(),
		   m_head.angleY() );
    m_json.update( "setspeed",
		   m_leftwheel .setspeed(),
		   m_rightwheel.setspeed() );
    m_json.update( "power",
		   m_leftwheel .power(),
		   m_rightwheel.power() );
    m_json.update( "speed",
		   m_leftwheel .speed(),
		   m_rightwheel.speed() );
    m_json.update( "counts",
		   m_leftwheel .count(),
		   m_rightwheel.count() );
    m_json.update( "pid",
		   m_leftwheel.pid().Kp(),
		   m_leftwheel.pid().Ki(),
		   m_leftwheel.pid().Kd() );

    m_json.send();
}

bool Robot::processjson( char *line )
{
    MyJson json( line );

    if (!json.opendict()) {
	Serial.print( "MyJson - unable to open dict: " );
	Serial.println( line );
	return false;
    }
    char *key;
    while ((key = json.getkey()) != NULL) {
	if        (!strcmp( key, "head" )) {
	    double x = json.fgetvalue();
	    double y = json.fgetvalue();
	    if (!json.error()) {
		look( x, y );
		m_json.update( "head",
			       m_head.angleX(),
			       m_head.angleY() );
	    }
	} else if (!strcmp( key, "setspeed" )) {
	    double x = json.fgetvalue();
	    double y = json.fgetvalue();
	    if (!json.error()) {
		run( x, y );
		m_json.update( "setspeed",
			       m_leftwheel .setspeed(),
			       m_rightwheel.setspeed() );
	    }
	} else if (!strcmp( key, "power" )) {
	    double x = json.fgetvalue();
	    double y = json.fgetvalue();
	    if (!json.error()) {
		setpower( x, y );
		m_json.update( "power",
			       m_leftwheel .power(),
			       m_rightwheel.power() );
	    }
	} else if (!strcmp( key, "pid" )) {
	    double Kp = json.fgetvalue();
	    double Ki = json.fgetvalue();
	    double Kd = json.fgetvalue();
	    if (!json.error()) {
		m_leftwheel .pid().setPID( Kp, Ki, Kd );
		m_rightwheel.pid().setPID( Kp, Ki, Kd );
		m_updated = true;
		m_json.update( "pid",
			       m_leftwheel.pid().Kp(),
			       m_leftwheel.pid().Ki(),
			       m_leftwheel.pid().Kd() );
	    }
	} else {
	    Serial.print( "MyJson - unknown key: " );
	    Serial.println( key );
	}
	if (json.error()) {
	    Serial.print( "MyJson - error:" );
	    Serial.print( key );
	    Serial.println( ":" );
	    return false;
	}
    }
    if (json.error()) {
	Serial.println( "MyJson - error processing line." );
	return false;
    }
    // m_json.send();

    return true;
}

// Local Variables:
// c-basic-offset: 4
// End: 
