// -*- c++ -*-

#ifndef SETUP_H
#define SETUP_H

#include <ArduinoJson.h>

void SetupRobot();
void ConfigRobot( JsonObject& config );
void LoopRobot();

bool RobotCommand( char* line );

#endif //SETUP_H

// Local Variables:
// c-basic-offset: 4
// End: 
