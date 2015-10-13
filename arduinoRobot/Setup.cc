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
    2,				//   Arduino input pin for the encoder signal
    0,				//   Arduino interrupt number to generate
    leftEncoderISR		//   Interrupt service routine for this encoder
    );

Encoder rightencoder(		// Right wheel encoder
    3,				//   Arduino input pin for the encoder signal
    1,				//   Arduino interrupt number to generate
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

void SetupRobot()
{
    // Initialise the I2C wire comms with the Raspberry Pi
    //int i2c_slave_address = 0x4;
    //SetupComms( i2c_slave_address );
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
