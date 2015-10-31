// -*- c++ -*-

#include <Arduino.h>
#include <Wire.h>

#include "Comms.h"
#include "Setup.h"

#define BUFLEN 128

static char  commandline[BUFLEN];
static char* nextchar = commandline;
static bool  commandavailable = false;

// Read whatever characters are ready from the RaspberryPi. 
// If we have a whole line of text, return the line of text
// Else if we don't have a whole line,
//    ...save what we have read so far and return NULL
char* ReadCommandLine()
{
    char c;
    // While there are more characters to be read from the port
    while (Wire.available()) {
	c = Wire.read();            // Read the next character
	if (c != '\n') {            // If this is not the end of the line...
	    *nextchar++ = c;        // Add the next character to the string
	    *nextchar = '\0';       // Terminate the string
	} else {                    // If this is the end of the line...
	    *nextchar = '\0';       // Terminate the string
	    nextchar = commandline; // Reset nextchar back to the start
	    commandavailable = true;
	    return commandline;     // Return the commandline
	}
    }
    return NULL;
}

char* ReadCommand_serial()
{
    char c;
    // While there are more characters to be read from the port
    while (Serial.available()) {
	// digitalWrite( 13, HIGH ); // Flash the LED
	c = Serial.read();          // Read the next character
	if (c != '\n') {            // If this is not the end of the line...
	    *nextchar++ = c;        // Add the next character to the string
	    *nextchar = '\0';       // Terminate the string
	} else {                    // If this is the end of the line...
	    *nextchar = '\0';       // Terminate the string
	    nextchar = commandline; // Reset nextchar back to the start
	    commandavailable = true;
	    return commandline;     // Return the commandline
	}
    }
    return NULL;
}

// status =  0 if command in progress
// status =  1 if command completed successfully
// status =  2 if command error
static volatile int cmdstatus = 1;

char *ReadCommand()
{
    static char buffer[BUFLEN];
  
    if (!commandavailable) {
	return NULL;
    }
    cmdstatus = 1;
    strncpy( buffer, commandline, BUFLEN );
    buffer[BUFLEN-1] = '\0';
    commandavailable = false;
    return buffer;
}

// Callback function for sending data to the RaspberryPi over the i2c bus
void sendData()
{
    Wire.write( cmdstatus );
}

// Callback function to process data sent from the the RaspberryPi over i2c
void receiveData(int byteCount)
{
    cmdstatus = 0;
    // Try to read an entire command line
    char *s = ReadCommandLine();
    if (s == NULL) {
	cmdstatus = 0;	// Didn't get the whole cmd yet: waiting for the rest
    } else  {
	cmdstatus = 1;
    }
}

void SetupComms( int i2c_slave_address )
{
    Wire.begin( i2c_slave_address ); // Setup as a slave on the i2c bus
    Wire.onReceive( receiveData );   // Receive commands from RPI
    Wire.onRequest( sendData );      // Send data when asked
}

void SetupComms_serial( )
{
    // Flush any incoming data in the serial port
    while (Serial.available()) {
	Serial.read();          // Read the next character
    }
}


// Local Variables:
// c-basic-offset: 4
// End: 
