// -*- c++ -*-
#include <Arduino.h>

#include "Setup.h"

static const int LED_PIN = 13;

void setup()
{
  pinMode( LED_PIN, OUTPUT );          // To control the Arduino LED
  Serial.begin( 9600 );           // For printing diagnostic outputs

  SetupRobot();
}

void loop()
{
  static int  tick       = 0;
  static bool ledstate   = false;
  // Flash the LED several times per second for the first 5 seconds.
  static int  halfperiod = 200;

  if (tick == 5000) {
    // After 5 seconds flash the LED slowly.
    // This is useful to check the arduino has not crashed.
    halfperiod = 1000;
  }
  if (tick % halfperiod == 0) {
    ledstate = ! ledstate;
    digitalWrite( LED_PIN, (ledstate ? HIGH : LOW) ); // Flash the LED on/off
  }

  LoopRobot();

  delay( 1 );
  tick++;
}

// Local Variables:
// c-basic-offset: 2
// End: 
