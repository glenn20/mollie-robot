// -*- c++ -*-

#include <string.h>

#include <PID_v1.h>
#include <ArduinoJson.h>
#include <MemoryFree.h>

#include "Robot.h"
#include "Setup.h"

Robot::Robot( Wheel&       leftwheel,
	      Wheel&       rightwheel,
	      Head&        head
    )
    :         m_leftwheel  ( leftwheel ),
	      m_rightwheel ( rightwheel ),
	      m_head       ( head ),
	      m_state      ( ),
	      m_updated    ( false )
{
}

void Robot::close()
{
    m_leftwheel.close();
    m_rightwheel.close();
    m_head.close();
}

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
    bool updated = false;
    //Serial.println( F("RobotCommand():") );
    //Serial.flush();
    
    if (line[0] == '{') {
	updated = processjson( line );
    } else {
	Serial.print( "Unknown robot command: " ); Serial.println( line );
    }

    return updated;
}

bool Robot::Loop()
{
    static unsigned long lasttime = 0;

    bool leftupdate = m_leftwheel.Loop();
    bool rightupdate = m_rightwheel.Loop();

    unsigned long thistime = millis();

    if ((thistime - lasttime > 500) &&
	((leftupdate || rightupdate) ||
	 (m_leftwheel.encoder().moving() || m_rightwheel.encoder().moving()))) {
	m_state.speed  = true;
	m_state.power  = true;
	m_state.counts = true;
	m_updated = true;
    }

    if (sendstate()) {
	Serial.flush();
	lasttime = thistime;
    }

    bool status = m_updated;
    m_updated = false;

    return status;
}

bool Robot::processtarget( JsonObject& d )
{
    for (JsonObject::iterator it=d.begin(); it != d.end(); ++it) {
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
	    Serial.print( F("processtarget() - unknown key: ") );
	    Serial.println( key );
	}
    }

    return true;
}

bool Robot::processjson( const char *json )
{
    if (freeMemory() < 100) {
	Serial.print( F("Warning: processjson(): Free SRAM low (Bytes) = ") );
	Serial.println( freeMemory() );
    }

    // The internal buffer for the Json objects
    StaticJsonBuffer<300> jsonBuffer;

    JsonObject& root = jsonBuffer.parseObject( json );

    if (!root.success()) {
	Serial.print( "Processing Json: parseObject() failed:" );
	Serial.println( json );
	return false;
    }

    for (JsonObject::iterator it=root.begin(); it != root.end(); ++it) {
	const char* key = it->key;
	if        (!strcmp( key, "target" )) {
	    JsonObject& value = it->value;
	    processtarget( value );
	} else if (!strcmp( key, "track" )) {
	    double x = (it->value)[0];
	    double y = (it->value)[1];
	    dotrackingPID( x, y );
	} else if (!strcmp( key, "config" )) {
	    JsonObject& config = it->value;
	    ConfigRobot( config );
	    m_state.config = true;
	} else {
	    Serial.print( F("ProcessJson() - unknown key ignored: ") );
	    Serial.println( key );
	}
    }

    return true;
}

bool Robot::sendstate()
{
    DynamicJsonBuffer jsonBuffer;

    JsonObject& root = jsonBuffer.createObject();
    JsonArray* data = NULL;

    if (m_state.head) {
	m_state.head = false;
	data = &root.createNestedArray("head");
	data->add( m_head.angleX() );
	data->add( m_head.angleY() );
    }
    if (m_state.power) {
	m_state.power = false;
	data = &root.createNestedArray("power");
	data->add( m_leftwheel .power() );
	data->add( m_rightwheel.power() );
    }
    if (m_state.setspeed) {
	m_state.setspeed = false;
	data = &root.createNestedArray("setspeed");
	data->add( m_leftwheel .setspeed() );
	data->add( m_rightwheel.setspeed() );
    }
    if (m_state.speed) {
	m_state.speed = false;
	data = &root.createNestedArray("speed");
	data->add( m_leftwheel .speed() );
	data->add( m_rightwheel.speed() );
    }
    if (m_state.counts) {
	m_state.counts = false;
	data = &root.createNestedArray("counts");
	data->add( m_leftwheel .count() );
	data->add( m_rightwheel.count() );
    }
    if (m_state.pid) {
	m_state.pid = false;
	data = &root.createNestedArray("pid");
	data->add( m_leftwheel.pid().Kp() );
	data->add( m_leftwheel.pid().Ki() );
	data->add( m_leftwheel.pid().Kd() );
    }

    if (data != NULL) {
	root["time"] = millis() / 1000.0;
	root.printTo( Serial );
	Serial.println();
	if (freeMemory() < 100) {
	    Serial.print( F("Warning: Free SRAM low (Bytes) = ") );
	    Serial.println( freeMemory() );
	}
	Serial.flush();
	return true;
    }

    return false;
}

// Local Variables:
// c-basic-offset: 4
// End: 
