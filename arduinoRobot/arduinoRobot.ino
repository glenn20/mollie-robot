// -*- c++ -*-
#include <Wire.h>
#include <Servo.h>
#include <PID_v1.h>

#include "Commands.h"

void setup()
{
    pinMode(13, OUTPUT);          // To control the Arduino LED
    Serial.begin(9600);           // For printing diagnostic outputs
    SetupCommands();
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
	    digitalWrite(13, HIGH); // set the LED on
	    ledstate = 1;
	}else{
	    digitalWrite(13, LOW); // set the LED off
	    ledstate = 0;
	}
    }
    LoopCommands();

    delay(1);
    tick++;
}

// Local Variables:
// c-basic-offset: 2
// End: 
