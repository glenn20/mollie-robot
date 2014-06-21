// -*- c++ -*-
#include <Arduino.h>

#include "Setup.h"

void setup()
{
    pinMode( 13, OUTPUT );          // To control the Arduino LED
    Serial.begin( 9600 );           // For printing diagnostic outputs

    SetupRobot();
}

void loop()
{
    static int tick     = 0;
    static int ledstate = 0;
    static int period   = 200;

    if (tick == 5000) {
	period = 1000;
    }
    if (tick % period == 0) {
	if (ledstate == 0) {
	    digitalWrite( 13, HIGH ); // set the LED on
	    ledstate = 1;
	}else{
	    digitalWrite( 13, LOW ); // set the LED off
	    ledstate = 0;
	}
    }

    LoopRobot();

    delay( 1 );
    tick++;
}

// Local Variables:
// c-basic-offset: 2
// End: 
