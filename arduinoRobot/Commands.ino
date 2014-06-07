// -*- c++ -*-

#include <Wire.h>
#include <Servo.h>
#include <PID_v1.h>

#include "RobotMotors.h"

#define SLAVE_ADDRESS 0x04

// Define the initialisation parameters for the Motors, Encoders and Servos

// Forward declaration of the Interrupt Service Routines for the Encoders
void leftEncoderISR();
void rightEncoderISR();

// Left  Encoder: Arduino Pin for the encoder signal, and interrupt number
EncoderParams leftencoder ( 3, 1, leftEncoderISR );
// Right Encoder: Arduino Pin for the encoder signal, and interrupt number
EncoderParams rightencoder( 2, 0, rightEncoderISR);
// Left motor: controlpin1, controlpin2, enablePin, leftencoder
MotorParams   leftwheel   ( 7, 4,  6, leftencoder );
// Left motor: controlpin1, controlpin2, enablePin, leftencoder
MotorParams   rightwheel  ( 5, 8, 11, rightencoder );
// Head: Arduino pin for the Servo control pin
HeadParams    head( 10 );
// Robot: Is built from two wheels and a head
RobotParams   robotparams( leftwheel, rightwheel, head );

// Construct an instance of our Robot initialised with the Parameters above
Robot robbie( robotparams );

void leftEncoderISR()  {robbie.leftwheel.encoder.update();}
void rightEncoderISR() {robbie.rightwheel.encoder.update();}

// --- Init PID Controller ---
double posX = 0.0;

//Define Variables we'll be connecting to
double SetpointX, InputX, OutputX;
double Setpointbody, Inputbody, Outputbody;

//Specify the links and initial tuning parameters
// face tracking: 0.8, 0.6, 0
// color tracking: 0.4, 0.4, 0
PID myPIDX(&InputX, &OutputX, &SetpointX, 0.1, 0.0, 0.0, DIRECT);
PID myPIDbody(&Inputbody, &Outputbody, &Setpointbody, 5.0, 0.0, 0.0, DIRECT);

char  commandline[128];
char* nextchar = commandline;

// Read whatever characters are ready from the RaspberryPi. 
// If we have a whole line of text, return the line of text
// Else if we don't have a whole line,
//    ...save what we have read so far and return NULL
char* ReadCommandLine()
{
  char c;
  // While there are more characters to be read from the serial port
  while (Wire.available()) {
    c = Wire.read();            // Read the next character
    if (c != '\n') {            // If this is not the end of the line...
      *nextchar++ = c;        // Add the next character to the string
      *nextchar = '\0';       // Terminate the string
    } else {                    // If this is the end of the line...
      *nextchar = '\0';       // Terminate the string
      nextchar = commandline; // Reset nextchar back to the start
      return commandline;     // Return the commandline
    }
  }
  return NULL;
}


// Process a line of text containing commands to move the motors
// Returns true if any action was taken, else return false.
boolean RobotCommand( char* line )
{
  char command[20] = "";// A string to hold the motor command
  int numbers[8];          // The list of numbers following the command

  // Every command starts with the <command>, then up to eight integer numbers
  int n = sscanf( line, "%20s %d %d %d %d %d %d %d %d", 
		  command, 
		  &numbers[0], &numbers[1], &numbers[2], &numbers[3], 
		  &numbers[4], &numbers[5], &numbers[6], &numbers[7] );
  String cmd = command;
  n--;
  if (cmd == "run" && n == 2) {
    // For all the motor numbers provided, set them to the speed provided
    //   run 235 -235"
    int speed = numbers[0];
    int direction = numbers[1];
    robbie.Go( speed, direction );
  } else if (cmd == "setdirection" && n == 1) {
    // Set the motor directions
    // 1 means Forward, 0 means Backward
    //   setdirection 0
    robbie.Go( robbie.speed(), numbers[0] );
  } else if (cmd == "servo" && n == 1) {
    // set servo angle
    int angle = numbers[0];
    if (-90 <= angle && angle <= 90) {
      robbie.look( angle );
    }
  } else if (cmd == "enable" && n == 0) {
    // Enable both motors
    robbie.enable();
  } else if (cmd == "disable" && n == 0) {
    // Disable both motors
    robbie.disable();
  } else {
    return false;
  }
  return true;
}

boolean TrackCommand( char* line )
{
  char command[20] = "";   // A string to hold the servo command
  int numbers[8];          // The list of numbers following the command

  int n = sscanf( line, "%20s %d %d %d %d %d %d %d %d", 
		  command, 
		  &numbers[0], &numbers[1], &numbers[2], &numbers[3], 
		  &numbers[4], &numbers[5], &numbers[6], &numbers[7] );
  String cmd = command;
  n--;
  if (cmd == "track" && n == 2) {
    // set servo angle
    // servo 0-180
    InputX = numbers[0];
    //InputY = numbers[1];
    myPIDX.Compute();
    if (OutputX < -5.0 || OutputX > -5.0) { 
      // Update Servo Position
      posX = constrain(posX + OutputX, -90, 90);
      //Serial.print( "Camera direction: " );
      //Serial.println( posX );
      robbie.look( posX );

      // Now, turn the body toward where the camera is looking
      Inputbody = posX;
      myPIDbody.Compute();
        
      if (Outputbody < -10.0 || Outputbody > 10.0) {
	int direction = constrain( Outputbody, -100, 100 );
	//Serial.print( "Robot direction: " );
	//Serial.println( direction );
	robbie.Go( robbie.speed(), direction );
      }
    }
  } else {
    return false;
  }
  return true;
}

void SetupCommands()
{
  Wire.begin(SLAVE_ADDRESS);    // Setup as a slave on the i2c bus
  Wire.onReceive(receiveData);  // Receive commands from RPI
  Wire.onRequest(sendData);     // Send data when asked

  // Initialise the Robot motors
  robbie.initialise();
  robbie.enable();
  delay(1);

  // --- Setup PID ---
  SetpointX = 0;
  Setpointbody = 0;
  myPIDX.SetOutputLimits(-90, 90);
  myPIDbody.SetOutputLimits(-100, 100);
  //turn PIDs on
  myPIDX.SetMode(AUTOMATIC);
  myPIDbody.SetMode(AUTOMATIC);
}

void LoopCommands()
{
  robbie.Loop();
}

// status =  0 if command in progress
// status =  1 if command completed successfully
// status =  2 if command error
static int cmdstatus = 1;

// Callback function to process data sent from the the RaspberryPi over i2c
void receiveData(int byteCount)
{
  cmdstatus = 0;
  // Try to read an entire commend line
  char *s = ReadCommandLine();
  if (s == NULL) {
    cmdstatus = 0;	// Didn't get the whole cmd yet: waiting for the rest
  } else if (RobotCommand( s ) == true) {
    cmdstatus = 1;
  } else if (TrackCommand( s ) == true) {
    cmdstatus = 1;
  } else {
    cmdstatus = 2;
  }
}

// Callback function for sending data to the RaspberryPi over the i2c bus
void sendData()
{
  Wire.write( cmdstatus );
}

// Local Variables:
// c-basic-offset: 2
// End: 
