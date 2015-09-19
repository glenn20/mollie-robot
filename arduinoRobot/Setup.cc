// -*- c++ -*-

#include <Arduino.h>
#include <PID_v1.h>

#define USEAFMOTOR

#if defined(USEMICROM)
// The Dagu MicroMagician Motor Control library
#include <microM.h>
#endif
#if defined(USEAFMOTOR)
// The AdaFruit Motor Shield Library
#include <AFMotor.h>
#endif

#include "Setup.h"
#include "Comms.h"

#include "Encoder.h"
#include "Motor.h"
#include "Wheel.h"
#include "Head.h"
#include "Robot.h"

// Use the Mollie implementation of the Motor class
#if defined(USEMICROM)
#include "Motor-MicroM.h"
#elif defined(USEAFMOTOR)
#include "Motor-AFMotor.h"
#else
#include "Motor-Mollie.h"
#endif

// Define the initialisation parameters for the Motors, Encoders and Servos

// Forward declaration of the Interrupt Service Routines for the Encoders
void leftEncoderISR();
void rightEncoderISR();

// Start assembling Robbie the robot from the ground up...
Encoder leftencoder (		// Left wheel encoder
    3,				//   Arduino input pin for the encoder signal
    1,				//   Arduino interrupt number to generate
    leftEncoderISR		//   Interrupt service routine for this encoder
    );

Encoder rightencoder(		// Right wheel encoder
    2,				//   Arduino input pin for the encoder signal
    0,				//   Arduino interrupt number to generate
    rightEncoderISR		//   Interrupt service routine for this encoder
    );

#if defined(USEMICROM)
MotorMicroM leftmotor(		// Left wheel DC motor controller
    true			//   This is the left motor on the controller
    );
MotorMicroM rightmotor(		// Right wheel DC motor controller
    false			//   This NOT the left motor on the controller
    );
#elif defined(USEAFMOTOR)
MotorAFMotor leftmotor(		// Left wheel DC motor controller
    3				//   Left wheel attached to Motor 3
    );
MotorAFMotor rightmotor(	// Right wheel DC motor controller
    4				//   Right wheel attached to Motor 4
    );
#else
MotorMollie leftmotor(		// Left wheel DC motor controller
    7,				// Arduino pin for first  H-Bridge control pin
    4,				// Arduino pin for second H-Bridge control pin
    6				// Arduino PWN pin for H-Bridge Enable pin
    );
MotorMollie rightmotor(		// Right wheel DC motor controller
    5,				// Arduino pin for first  H-Bridge control pin
    8,				// Arduino pin for second H-Bridge control pin
    11				// Arduino PWM pin for the H-Bridge Enable pin
    );
#endif

Wheel leftwheel   (		// Left wheel DC motor and encoder
    leftmotor,
    leftencoder,
    "Leftwheel"
    );

Wheel rightwheel  (		// Right wheel DC motor and encoder
    rightmotor,
    rightencoder,
    "Rightwheel"
    );

HeadServo headX (
    10,				// Arduino PWM pin for the rotation servo
    68,				//   origin: Servo setting for straight ahead
    18,				//   min: Minimum servo setting
    116				//   max: Maximum servo setting
    );

HeadServo headY (
    9,				// Arduino PWM pin for the rotation servo
    54,				//   origin: Servo setting for straight ahead
    24,				//   min: Minimum servo setting
    106				//   max: Maximum servo setting
    );
  
Head  head        (		// The Robot head is built from two Servos
    headX,			//   The horizontal rotation servo, and...
    headY			//   The vertical rotation servo
    );

Robot robbie      (		// Robbie the robot is built from...
    leftwheel,			//   A Left wheel,
    rightwheel,			//   A Right wheel and...
    head			//   A Head made of servos.
    );

// Load the interrupt service routines for the left and right wheel encoders
void leftEncoderISR()  {robbie.leftwheel().encoder().update(); }
void rightEncoderISR() {robbie.rightwheel().encoder().update();}

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

void SetupRobot()
{
    // Initialise the I2C wire comms with the Raspberry Pi
    int i2c_slave_address = 0x4;
    SetupComms( i2c_slave_address );

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

void LoopRobot()
{
    // If a command is available - process it...
    char *s = ReadCommand();
    if (s != NULL) {
	RobotCommand( s );
    }

    // Update the Robot...
    robbie.Loop();
}

// Process a line of text containing commands to move the motors
// Returns true if any action was taken, else return false.
bool RobotCommand( char* line )
{
    char command[20] = "";// A string to hold the command
    int  numbers[8];      // The list of numbers following the command

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
	robbie.run( speed, direction );
    } else if (cmd == "setpower" && n == 2) {
	// For all the motor numbers provided, set them to the speed provided
	//   power 235 -235"
	int power1 = numbers[0];
	int power2 = numbers[1];
	robbie.leftwheel().setpower( power1 );
	robbie.rightwheel().setpower( power2 );
    } else if (cmd == "setdirection" && n == 1) {
	// Set the motor directions
	// 1 means Forward, 0 means Backward
	//   setdirection 0
	robbie.run( robbie.speed(), numbers[0] );
    } else if (cmd == "look" && n == 2) {
	// set servo angle for the robot head - and camera
	int anglex = numbers[0];
	int angley = numbers[1];
	robbie.look( anglex, angley );
    } else if (cmd == "enable" && n == 0) {
	// Enable both motors
	robbie.enable();
    } else if (cmd == "disable" && n == 0) {
	// Disable both motors
	robbie.disable();
    } else if (cmd == "track" && n == 2) {
	// set servo angle
	// servo 0-180
	InputX = numbers[0];
	//InputY = numbers[1];
	myPIDX.Compute();
	if (-5.0 < OutputX && OutputX < 5.0) {
	    // Accept small angles
	    return false;
	}
	// Update Servo Position
	posX = constrain(posX + OutputX, -90, 90);
	//Serial.print( "Camera direction: " );
	//Serial.println( posX );
	robbie.look( posX );
    
	// Now, turn the body toward where the camera is looking
	Inputbody = posX;
	myPIDbody.Compute();
    
	if (-5.0 < Outputbody && Outputbody < 5.0) {
	    return true;
	}
	int direction = constrain( Outputbody, -100, 100 );
	//Serial.print( "Robot direction: " );
	//Serial.println( direction );
	robbie.run( robbie.speed(), direction );
    }
    return true;
}

// Local Variables:
// c-basic-offset: 4
// End: 
