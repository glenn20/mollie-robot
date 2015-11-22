// -*- c++ -*-

#ifndef HEAD_H
#define HEAD_H

#include <Servo.h>
#include <PID_v1.h>

// A helper class for the robot "Head"
// This class trivially wraps the Arduino Servo class.
class HeadServo {
public:
    HeadServo();

    void  initialise( int servopin,
		      int min    = 0,
		      int origin = 90,
		      int max    = 180 );

    void  close();
    float setangle( float angle );
    float angle();

private:
    Servo m_servo;
    float m_angle;
    int   m_servopin;
    int   m_origin;
    int   m_min;
    int   m_max;
};

// A Head object represents the head of the robot
// - which has a camera mounted
// - and two servos to turn horizontally and vertically (X and Y)
// The robot will adjust the servos to "look" in the requested direction
// If no servopin is provided for the Y servo - assume our Head can only
// turn horizontally
class Head {
public:
    Head(
	HeadServo&  servox,
	HeadServo&  servoy
	);

    void     close();
    float    lookX( float angleX );  // Turn head to the given angle
    float    lookY( float angleY );  // Turn head to the given angle
    float    angleX();               // Get the current head turn angle
    float    angleY();
    void     look(  float angleX,
		    float angleY );

    float    look(  float angleX ) { return lookX( angleX ); };

private:
    HeadServo&   m_servoX;
    HeadServo&   m_servoY;
    //MyPID  m_pid;
};

#endif //HEAD_H

// Local Variables:
// c-basic-offset: 4
// End: 
