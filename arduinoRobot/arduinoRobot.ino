// -*- c++ -*-
#include <Arduino.h>

#include "Setup.h"

// The Arduino output pin which is used to blink the onboard LED.
static const int LED_PIN = 13;

void setup()
{
    pinMode( LED_PIN, OUTPUT );		// To control the Arduino LED
    Serial.begin( 115200 );		// For printing diagnostic outputs

    SetupRobot();
}

void loop()
{
    static long  starttime = 0;
    // Flash the LED several times per second for the first 5 seconds.
    static long  nexton   = 0;
    static long  nextoff  = 0;

    if (starttime == 0) {
	starttime = millis();
    }

    long tick = millis() - starttime;

    // After 5 seconds flash the LED slowly.
    int onperiod = (tick < 5000) ? 400 : 2000;
    int offperiod = 200;
    if (tick > nexton) {
	nexton  = tick + onperiod;
	nextoff = tick + offperiod;
	digitalWrite( LED_PIN, HIGH ); // Flash the LED
    } else if (tick > nextoff) {
	nextoff = tick + offperiod;
	digitalWrite( LED_PIN, LOW ); // Flash the LED
    }

    LoopRobot();

    //delay( 1 );
}

// Local Variables:
// c-basic-offset: 4
// End: 
