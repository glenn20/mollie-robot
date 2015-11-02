// -*- c++ -*-

#include <PID_v1.h>

#include <string.h>

#include <ArduinoJson.h>

#include <MemoryFree.h>

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
	m_state.speed = true;;
	m_state.counts = true;
    }
    if (sendstate())
	Serial.flush();

    bool status = m_updated;
    m_updated = false;

    return status;
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

bool Robot::processjson( char *json )
{
    // The internal buffer for the Json objects
    StaticJsonBuffer<150> jsonBuffer;

    JsonObject& root = jsonBuffer.parseObject( json );
    
    if (!root.success()) {
	Serial.print( "Processing Json: parseObject() failed:" ); Serial.println( json );
	return false;
    }

    for (JsonObject::iterator it=root.begin(); it != root.end(); ++it) {
	const char* key = it->key;
	if        (!strcmp( key, "head" )) {
	    double x = (it->value)[0];
	    double y = (it->value)[1];
	    look( x, y );
	    m_state.head = true;
	} else if (!strcmp( key, "setspeed" )) {
	    double x = (it->value)[0];
	    double y = (it->value)[1];
	    run( x, y );
	    m_state.setspeed = true;
	} else if (!strcmp( key, "power" )) {
	    double x = (it->value)[0];
	    double y = (it->value)[1];
	    setpower( x, y );
	    m_state.power = true;
	} else if (!strcmp( key, "pid" )) {
	    double Kp = (it->value)[0];
	    double Ki = (it->value)[1];
	    double Kd = (it->value)[2];
	    m_leftwheel .pid().setPID( Kp, Ki, Kd );
	    m_rightwheel.pid().setPID( Kp, Ki, Kd );
	    m_state.pid = true;
	    m_updated   = true;
	} else {
	    Serial.print( F("ProcessJson() - unknown key: ") );
	    Serial.println( key );
	}
	// if (key == "head") {
	//     look( (it->value)[0], (it->value)[1] );
	// } else if (key == "setspeed") {
	//     run( (it->value)[0], (it->value)[1] );
	// } else if (key == "power") {
	//     setpower( (it->value)[0], (it->value)[1] );
	// } else if (key == "pid") {
	//     m_leftwheel .pid().setPID( (it->value)[0],
	// 			       (it->value)[1],
	// 			       (it->value)[2] );
	//     m_rightwheel.pid().setPID( (it->value)[0],
	// 			       (it->value)[1],
	// 			       (it->value)[2] );
	//     m_updated = true;
	// } // Silently ignore any other json keys - for now
    }

    return true;
}

bool Robot::sendstate()
{
    String s;
    s.reserve( 80 );

    if (m_state.head) {
	m_state.head = false;
	s += (String( ",\"head\":[" ) +
	      String( m_head.angleX(), 2 ) +
	      "," +
	      String( m_head.angleY(), 2 ) +
	      "]" );
    }
    if (m_state.power) {
	m_state.power = false;
	s += (String( ",\"power\":[" ) +
	      String( m_leftwheel .power(), 2 ) +
	      "," +
	      String( m_rightwheel.power(), 2 ) +
	      "]" );
    }
    if (m_state.setspeed) {
	m_state.setspeed = false;
	s += (String( ",\"setspeed\":[" ) +
	      String( m_leftwheel .setspeed(), 2 ) +
	      "," +
	      String( m_rightwheel.setspeed(), 2 ) +
	      "]" );
    }
    if (m_state.speed) {
	m_state.speed = false;
	s += (String( ",\"speed\":[" ) +
	      String( m_leftwheel .speed(), 2 ) +
	      "," +
	      String( m_rightwheel.speed(), 2 ) +
	      "]" );
    }
    if (m_state.counts) {
	m_state.counts = false;
	s += (String( ",\"counts\":[" ) +
	      String( m_leftwheel .count() ) +
	      "," +
	      String( m_rightwheel.count() ) +
	      "]" );
    }
    if (m_state.pid) {
	m_state.pid = false;
	s += (String( ",\"pid\":[" ) +
	      String( m_leftwheel.pid().Kp(), 2 ) +
	      "," +
	      String( m_leftwheel.pid().Ki(), 2 ) +
	      "," +
	      String( m_leftwheel.pid().Kd(), 2 ) +
	      "]" );
    }

    if (s.length() > 0) {
	Serial.println( String( "{\"time\":" ) +
			String( millis() / 1000.0, 3 ) +
			s + "}" );
	Serial.flush();
	Serial.print( F("Free SRAM (Bytes) = ") );
	Serial.println( freeMemory() );
	return true;
    }

    return false;
}

// Local Variables:
// c-basic-offset: 4
// End: 
