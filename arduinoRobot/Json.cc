// -*- c++ -*-

#include <Arduino.h>

#include "Json.h"

// A brain-dead minimal json parser
// - assumes input is correctly formed and no whitespace
// - assumes only one dict containing key/value pairs
// - values must be single value or a list of values
MyJson::MyJson( char* json )
    : m_endarray	( false ),
      m_enddict		( false ),
      m_error		( false ),
      m_json		( json ),
      m_s		( json )
{
}

bool MyJson::opendict()
{
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

bool MyJson::enddict()
{
    return m_enddict;
}

bool MyJson::error()
{
    return m_error;
}

char* MyJson::getkey()
{
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

char* MyJson::getvalue()
{
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

double MyJson::fgetvalue()
{
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

MyJsonSend::MyJsonSend()
    : m_firsttime  ( true ),
      m_json       ( ),
      m_s          ( m_json ),
      m_end	   ( m_json + BUFLEN )
{
}

void MyJsonSend::add( const char* s )
{
    while (*s != '\0' && m_s < m_end - 1) {
	*m_s++ = *s++;
    }
    *m_s = '\0';
}

void MyJsonSend::addi( int i )
{
    char sbuf[20];
    itoa( millis(), sbuf, 10 );
    add( sbuf );
}

void MyJsonSend::addui( unsigned long i )
{
    char sbuf[20];
    ultoa( millis(), sbuf, 10 );
    add( sbuf );
}

void MyJsonSend::addd( double d )
{
    char sbuf[20] = "";
    dtostrf( d, 1, 2, sbuf );
    add( sbuf );
}

void MyJsonSend::firsttime()
{
    if (m_firsttime) {
	m_firsttime = false;
	m_s = m_json;
	m_end = m_json + BUFLEN;
	add( "{\"time\":" );
	addui( millis() );
    }
}

void MyJsonSend::update( const char* key, double x1 )
{
    firsttime();
    add( ",\"" );
    add( key );
    add( "\":" );
    addd( x1 );
}

void MyJsonSend::update( const char* key, double x1, double x2 )
{
    firsttime();
    add( ",\"" );
    add( key );
    add( "\":[" );
    addd( x1 );
    add( "," );
    addd( x2 );
    add( "]" );
}

void MyJsonSend::update( const char* key, double x1, double x2, double x3 )
{
    firsttime();
    add( ",\"" );
    add( key );
    add( "\":[" );
    addd( x1 );
    add( "," );
    addd( x2 );
    add( "," );
    addd( x3 );
    add( "]" );
}

bool MyJsonSend::send()
{
    if (m_firsttime) {
	return false;
    }
    add( "}" );
    Serial.println( m_json );
    //Serial.flush();
    
    m_firsttime = true;
    return true;
}

// Local Variables:
// c-basic-offset: 4
// End: 
