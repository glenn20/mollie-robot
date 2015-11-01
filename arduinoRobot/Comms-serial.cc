// -*- c++ -*-

#include <Arduino.h>

#include "Comms-serial.h"
#include "Setup.h"

#define BUFLEN 200

static char  commandline[BUFLEN];
static char* nextchar = commandline;
static bool  commandavailable = false;

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
