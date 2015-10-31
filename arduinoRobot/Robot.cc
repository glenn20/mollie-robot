// -*- c++ -*-

#include <PID_v1.h>

#include <string.h>

#include "Robot.h"

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
	m_updated = false;
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
    // Hand crafted pseudo-json - cause the json library seems to crash
    Serial.print( "{\"time\":" );
    Serial.print( millis() );
    Serial.print( ",\"head\":[" );
    Serial.print( m_head.angleX(), 2 );
    Serial.print( "," );
    Serial.print( m_head.angleY(), 2 );
    Serial.print( "],\"setspeed\":[" );
    Serial.print( m_leftwheel .setspeed(), 2 );
    Serial.print( "," );
    Serial.print( m_rightwheel.setspeed(), 2 );
    Serial.print( "],\"power\":[" );
    Serial.print( m_leftwheel .power(), 2 );
    Serial.print( "," );
    Serial.print( m_rightwheel.power(), 2 );
    Serial.print( "],\"speed\":[" );
    Serial.print( m_leftwheel .speed(), 2 );
    Serial.print( "," );
    Serial.print( m_rightwheel.speed(), 2 );
    Serial.print( "],\"counts\":[" );
    Serial.print( m_leftwheel .encoder().count() );
    Serial.print( "," );
    Serial.print( m_rightwheel.encoder().count() );
    Serial.print( "],\"pid\":[" );
    Serial.print( m_leftwheel.pid().Kp(), 2 );
    Serial.print( "," );
    Serial.print( m_leftwheel.pid().Ki(), 2 );
    Serial.print( "," );
    Serial.print( m_leftwheel.pid().Kd(), 2 );
    Serial.println( "]}" );
    Serial.flush();
}

// A brain-dead minimal json parser
// - assumes input is correctly formed and no whitespace
// - assumes only one dict containing key/value pairs
// - values must be single value or a list of values
class MyJson {
public:
    MyJson( const char* json )
	: m_endarray	( false ),
	  m_enddict	( false ),
	  m_error	( false ),
	  m_s		( NULL ) {
	strncpy( m_json, json, sizeof( m_json ) / sizeof( m_json[0] ) );
	m_s = m_json;
    }

    bool opendict() {
	// Return if at end of string or error
	if (*m_s == '\0' || m_error)
	    return false;
	
	// Unset the flag for end of list of values...
	m_endarray = m_enddict = false;

	if (*m_s != '{') {
	    return false;
	}
	// Skip over the opening brace and return true.
	m_s++;

	return true;
    }

    bool enddict() {
	return m_enddict;
    }

    bool error() {
	return m_error;
    }

    char* getkey() {
	// Return if at end of string
	if (*m_s == '\0' || m_error || m_enddict)
	    return NULL;

	// Unset the flag for end of list of values...
	m_endarray = false;

	char *s = m_s;
	if (*s == '}') {
	    m_enddict = true;
	    return NULL;
	}

	// Expecting to find double-quotes around the key name
	if (*s++ != '"') {
	    m_error = true;
	    return NULL;
	}
	// Find the end of the key name
	char *t;
	for (t = s; *t != '"'; t++) {
	    if (*t == '\0') {
		// End of string - abort
		m_error = true;
		return NULL;
	    }
	}
	// Set the end of the string to the terminator
	*t++ = '\0';
	if (*t != ':') {
	    // Expect a ':' after the key name - else abort
	    m_error = true;
	    return NULL;
	}
	// Set the next char pointer to the next char...for next time.
	m_s = ++t;

	return s;
    }

    char* getvalue() {
	// End of string - abort
	if (*m_s == '\0' || m_error || m_enddict)
	    return NULL;
	// If we have hit the end of the list of values - return NULL
	if (m_endarray)
	    return NULL;

	char *s = m_s;
	if (*s == '[') {
	    // We have list of values...skip forward
	    s++;
	}
	char *t;
	// Read up to the next 'comma' (or end of list)..
	for (t = s; *t != ',' && *t != ']'; t++) {
	    if (*t == '\0') {
		// End of string - abort
		m_error = true;
		return NULL;
	    }
	}
	if (*t == ']') {
	    // At the end of a list of values...
	    // Set a flag so we return NULL all calls to getvalue
	    // .... until we do a getkey().
	    m_endarray = true;
	    // Overwrite the terminator (]) with end of string char
	    *t++ = '\0';
	    // Skip over a comma which might follow the end of list
	    if (*t == ',') {
		t++;
	    }
	} else {
	    // Overwrite the terminating comma with end of string char
	    *t++ = '\0';
	}
	m_s = t;

	return s;
    }

    double fgetvalue() {
	// Damn stupid avr-gcc generates relocation errors if I try to use atof()
	// or any of the following variants
	// return atof( s );
	// return String( s ).toFloat();
	// double f; sscanf( "0.0", "%lg", &f ); return f;
	// return 0.0;
	char* s = getvalue();
	if (s == NULL) {
	    return 0.0;
	}
	return strtod( s, NULL );
    }

private:
    bool  m_endarray;
    bool  m_enddict;
    bool  m_error;
    char* m_s;
    char  m_json[300];
};

bool Robot::processjson( const char *line )
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
	    }
	} else if (!strcmp( key, "setspeed" )) {
	    double x = json.fgetvalue();
	    double y = json.fgetvalue();
	    if (!json.error()) {
		run( x, y );
	    }
	} else if (!strcmp( key, "power" )) {
	    double x = json.fgetvalue();
	    double y = json.fgetvalue();
	    if (!json.error()) {
		setpower( x, y );
	    }
	} else if (!strcmp( key, "pid" )) {
	    double Kp = json.fgetvalue();
	    double Ki = json.fgetvalue();
	    double Kd = json.fgetvalue();
	    if (!json.error()) {
		m_leftwheel .pid().setPID( Kp, Ki, Kd );
		m_rightwheel.pid().setPID( Kp, Ki, Kd );
		m_updated = true;
	    }
	} else {
	    Serial.print( "MyJson - unknown key: " );
	    Serial.println( key );
	}
	if (json.error()) {
	    Serial.print( "MyJson - error:" );
	    Serial.print( key );
	    Serial.print( ":" );
	    Serial.println( line );
	    return false;
	}
    }
    if (json.error()) {
	Serial.print( "MyJson - error processing: " );
	Serial.println( line );
	return false;
    }
    
    return true;
}

// Local Variables:
// c-basic-offset: 4
// End: 
