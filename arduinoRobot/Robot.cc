// -*- c++ -*-

#include <PID_v1.h>

#include <string.h>

#include "Robot.h"

#include <ArduinoJson.h>


Robot::Robot( Wheel&       leftwheel,
	      Wheel&       rightwheel,
	      Head&        head
    )
    :         m_leftwheel  ( leftwheel ),
	      m_rightwheel ( rightwheel ),
	      m_head       ( head ),
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

    return true;
}

bool Robot::run( int left, int right )
{
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
	return processjson( line );
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
    // The internal buffer for the Json objects
    StaticJsonBuffer<200> jsonBuffer;

    JsonObject& root = jsonBuffer.createObject();
    JsonArray* data;

    root["time"]     = millis();
    data = &root.createNestedArray("head");
    data->add( m_head.angleX() );
    data->add( m_head.angleY() );
    data = &root.createNestedArray("setspeed");
    data->add( m_leftwheel .setspeed() );
    data->add( m_rightwheel.setspeed() );
    data = &root.createNestedArray("power");
    data->add( m_leftwheel .power() );
    data->add( m_rightwheel.power() );
    data = &root.createNestedArray("speed");
    data->add( m_leftwheel .speed() );
    data->add( m_rightwheel.speed() );
    data = &root.createNestedArray("counts");
    data->add( m_leftwheel .encoder().count() );
    data->add( m_rightwheel.encoder().count() );

    root.printTo( Serial );
    Serial.println();
}

bool Robot::processjson( char *json )
{
    // The internal buffer for the Json objects
    StaticJsonBuffer<200> jsonBuffer;

    JsonObject& root = jsonBuffer.parseObject( json );
    
    if (!root.success()) {
	Serial.print( "Processing Json: parseObject() failed:" ); Serial.println( json );
	return false;
    }

    for (JsonObject::iterator it=root.begin(); it != root.end(); ++it) {
	char k[100];
	strncpy( k, it->key, 100 );
	String key = k;
	if (key == "head") {
	    m_head.look( (it->value)[0], (it->value)[1] );
	} else if (key == "setspeed") {
	    run( (it->value)[0], (it->value)[1] );
	} else if (key == "power") {
	    setpower( (it->value)[0], (it->value)[1] );
	} // Silently ignore any other json keys - for now
    }

    return true;
}

// Local Variables:
// c-basic-offset: 4
// End: 
