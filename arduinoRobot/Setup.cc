// -*- c++ -*-

#include <Arduino.h>

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
    2,				//   Arduino input pin for the encoder signal
    leftEncoderISR		//   Interrupt service routine for this encoder
    );

Encoder rightencoder(		// Right wheel encoder
    3,				//   Arduino input pin for the encoder signal
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

// Construct a PID controller for the left Wheel
MyPID leftpid(
    0.3,			// Kp
    0.0,			// Ki
    0.0,			// Kd
    -255,			// Min output range
    255,			// Max output range
    200				// Milliseconds between re-computation and update
    );

// Construct a PID controller for the left Wheel
MyPID rightpid(
    0.3,			// Kp
    0.0,			// Ki
    0.0,			// Kd
    -255,			// Min output range
    255,			// Max output range
    200				// Milliseconds between re-computation and update
    );

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
HeadServo headX (
    10,				// Arduino PWM pin for the rotation servo
    68,				//   origin: Servo setting for straight ahead
    18,				//   min: Minimum servo setting
    116				//   max: Maximum servo setting
    );

// HeadY tilts head up and down
HeadServo headY (
    9,				// Arduino PWM pin for the rotation servo
    54,				//   origin: Servo setting for straight ahead
    24,				//   min: Minimum servo setting
    106				//   max: Maximum servo setting
    );

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


// Load the interrupt service routines for the left and right wheel encoders
void leftEncoderISR()  {robbie.leftwheel ().encoder().update(); }
void rightEncoderISR() {robbie.rightwheel().encoder().update(); }

void SetupRobot()
{
    // Initialise the comms with the Raspberry Pi
    SetupComms_serial();

    // Initialise the Robot motors
    robbie.initialise();
    robbie.enable();
    delay(1);
    Serial.println( "Robot ready" );
}

void LoopRobot()
{
    // If a command is available - process it...
    char *s = ReadCommand_serial();
    if (s != NULL) {
	robbie.robotcommand( s );
    }

    // Update the Robot...
    robbie.Loop();
}

// Local Variables:
// c-basic-offset: 4
// End: 
