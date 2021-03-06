// -*- c++ -*-

#include <Arduino.h>

#include <MemoryFree.h>
#include <ArduinoJson.h>

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
#include "Comms-serial.h"

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
    leftEncoderISR		//   Interrupt service routine for this encoder
    );

Encoder rightencoder(		// Right wheel encoder
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
MotorAFMotor leftmotor;	// Left wheel DC motor controller
MotorAFMotor rightmotor;	// Right wheel DC motor controller
#else
MotorMollie leftmotor(		// Left wheel DC motor controller
    7,				// Arduino pin for first  H-Bridge control pin
    4,				// Arduino pin for second H-Bridge control pin
    6				// Arduino PWM pin for H-Bridge Enable pin
    );
MotorMollie rightmotor(		// Right wheel DC motor controller
    5,				// Arduino pin for first  H-Bridge control pin
    8,				// Arduino pin for second H-Bridge control pin
    11				// Arduino PWM pin for the H-Bridge Enable pin
    );
#endif

// Construct a PID controller for the left Wheel
MyPID leftpid;

// Construct a PID controller for the left Wheel
MyPID rightpid;

// Construct the left "Wheel" from a Motor, an Encoder and a PID controller...
Wheel leftwheel   (
    leftmotor,
    leftencoder,
    leftpid,
    "Leftwheel"
    );

// Construct the right "Wheel" from a Motor, an Encoder and a PID controller...
Wheel rightwheel  (
    rightmotor,
    rightencoder,
    rightpid,
    "Rightwheel"
    );

// Construct the servo motor controllers for the robot's head
// HeadX turns head left and right
HeadServo headX;

// HeadY tilts head up and down
HeadServo headY;

// Construct the Head controller from the two servo controllers
Head  head        (
    headX,			//   The horizontal rotation servo, and...
    headY			//   The vertical rotation servo
    );

// Robbie the Robot is built from a left Wheel, a right Wheel and a Head
Robot robbie      (
    leftwheel,			//   A Left wheel,
    rightwheel,			//   A Right wheel and...
    head			//   A Head made of servos.
    );

// How to construct robbie in one statement...
// Robot robbie2 (
//     new Wheel(
// 	MotorAFMotor(		// Left wheel DC motor controller
// 	    3			//   Left wheel attached to Motor 3
// 	    ),
// 	Encoder(		// Left wheel encoder
// 	    2,			//   Arduino input pin for the encoder signal
// 	    leftEncoderISR	//   Interrupt service routine for this encoder
// 	    ),
// 	"Leftwheel"
// 	),
//     Wheel(
// 	MotorAFMotor(		// Left wheel DC motor controller
// 	    4			//   Right wheel attached to Motor 4
// 	    ),
// 	Encoder(		// Right wheel encoder
// 	    3,			//   Arduino input pin for the encoder signal
// 	    rightEncoderISR	//   Interrupt service routine for this encoder
// 	    ),
// 	"Rightwheel"
// 	),
//     Head(
// 	HeadServo(
// 	    10,			// Arduino PWM pin for the rotation servo
// 	    68,			//   origin: Servo setting for straight ahead
// 	    18,			//   min: Minimum servo setting
// 	    116			//   max: Maximum servo setting
// 	    ),			//   The horizontal rotation servo, and...
// 	HeadServo(
// 	    9,			// Arduino PWM pin for the rotation servo
// 	    54,			//   origin: Servo setting for straight ahead
// 	    24,			//   min: Minimum servo setting
// 	    106			//   max: Maximum servo setting
// 	    )			//   The vertical rotation servo
// 	)
//     );

void ConfigRobot( JsonObject& config )
{
    if (config.containsKey("leftwheel")) {
	JsonObject& d = config["leftwheel"];
	if (d.containsKey("AFMotor")) {
	    leftmotor.initialise(
		d["AFMotor"]["motornum"] );
	}
	if (d.containsKey("encoder")) {
	    leftencoder.initialise(
		d["encoder"]["controlpin"] );
	}
	if (d.containsKey("pid")) {
	    JsonObject& d2 = d["pid"];
	    if (d2.containsKey("K")) {
		leftpid.setPID( d2["K"][0], d2["K"][1], d2["K"][2] );
	    }
	    if (d2.containsKey("range")) {
		leftpid.setlimits( d2["range"][0],
				   d2["range"][1] );
	    }
	    if (d2.containsKey("sampletime_ms")) {
		leftpid.setSampletime( d2["sampletime_ms"] );
	    }
	}
    }
    
    if (config.containsKey("rightwheel")) {
	JsonObject& d = config["rightwheel"];
	if (d.containsKey("AFMotor")) {
	    rightmotor.initialise(
		d["AFMotor"]["motornum"] );
	}
	if (d.containsKey("encoder")) {
	    rightencoder.initialise(
		d["encoder"]["controlpin"] );
	}
	if (d.containsKey("pid")) {
	    JsonObject& d2 = d["pid"];
	    if (d2.containsKey("K")) {
		rightpid.setPID( d2["K"][0], d2["K"][1], d2["K"][2] );
	    }
	    if (d2.containsKey("range")) {
		rightpid.setlimits( d2["range"][0],
				    d2["range"][1] );
	    }
	    if (d2.containsKey("sampletime_ms")) {
		rightpid.setSampletime( d2["sampletime_ms"] );
	    }
	}
    }

    if (config.containsKey("head")) {
	JsonObject& d = config["head"];
	if (d.containsKey("X")) {
	    headX.initialise(
		d["X"]["controlpin"],
		d["X"]["range"][0],
		d["X"]["range"][1],
		d["X"]["range"][2] );
	}
	if (d.containsKey("Y")) {
	    headY.initialise(
		d["Y"]["controlpin"],
		d["Y"]["range"][0],
		d["Y"]["range"][1],
		d["Y"]["range"][2] );
	}
    }
}

// Load the interrupt service routines for the left and right wheel encoders
void leftEncoderISR()  {robbie.leftwheel ().encoder().update(); }
void rightEncoderISR() {robbie.rightwheel().encoder().update(); }

static long freeram = 0;

void SetupRobot()
{
    // Initialise the comms with the Raspberry Pi
    SetupComms_serial();

    // Initialise the Robot motors
    Serial.print( F("Free SRAM (Bytes) = ") );
    freeram = freeMemory();
    Serial.println( freeram );
    delay(1);
    Serial.println( F("Robot ready for config") );
}

void LoopRobot()
{
    // If a command is available - process it...
    char *s = ReadCommand_serial();
    if (s != NULL) {
	robbie.robotcommand( s );
    }

    // Update the Robot...
    if (robbie.Loop()) {
	if (freeMemory() != freeram) {
	    // Warn about apparent memory leaks on each arduino loop.
	    freeram = freeMemory();
	    Serial.print( F("Warning: Free SRAM (Bytes) = ") );
	    Serial.println( freeMemory() );
	}
    }
}

// Local Variables:
// c-basic-offset: 4
// End: 
