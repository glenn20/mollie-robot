#!/usr/bin/env python

"""
An interface to manage an Arduino controlled robot.

Classes:
    ArduinoRobot: The manager of the Arduino robot.
"""

from __future__ import print_function
import itertools
import time
import json
import sys, tty, termios

import paho.mqtt.client as mqtt

class MqRobot( mqtt.Client ):
    """
    A simple wrapper to send robot status messages to the MQTT broker
    """
    def __init__( self, robot, clientid = None ):
        super( MqRobot, self ).__init__( clientid )
        self.robot        = robot
        self.on_connect   = self._on_connect
        self.on_subscribe = self._on_subscribe
        self.on_message   = self._on_message
        # self.on_log       = self._on_log

    def _on_connect( self, mqttc, obj, flags, rc ):
        print( "MQTT: Connected: rc: " + str( rc ), end="\r\n" )
        self.subscribe( "/mollie-robot/state", 1 )

    def _on_subscribe( self, mqttc, obj, mid, granted_qos ):
        print( "MQTT: Subscribed: " + str( mid ) + " " + str( granted_qos ) )

    def _on_message( self, mqttc, obj, msg ):
        print( "MQTT: " + msg.topic + " " +
               str( msg.qos ) + " " + str( msg.payload ) )
        # Update the robot state
        self.robot.robotstate.state( json.loads( msg.payload ) )

    def _on_log( self, mqttc, obj, level, string ):
        print( "MQTT: " +  string )

    def initialise( self ):
        self.connect_async( "192.168.0.30", 1883, 60 )
        self.loop_start()

    def close( self ):
        self.loop_stop()
        self.disconnect()

    def update( self, state ):
        return self.publish( "/mollie-robot/target", state, qos=0, retain=True )


class RobotState:
    def __init__( self ):
        self.time       = 0
        self.head       = [0,0]
        self.setspeed   = [0,0]
        self.speed      = [0,0]
        self.counts     = [0,0]
        self.power      = [0,0]
        self.pid        = [0.2, 0.0, 0.0 ]

    def update( self, s ):
        print( "Line =", s, end="\r\n" )
        d = json.loads( s )
        self.__dict__.update( d )
        # print( "Robot =", self.__dict__, end="\r\n" )
        return d

    def listofvalues( self ):
        return list(
            itertools.chain(
                [self.time],
                self.head,
                self.setspeed,
                self.speed,
                self.counts,
                self.power,
                self.pid
            )
        )
    
    def listofkeys( self ):
        return [
            "time",
            "headX", "headY",
            "setspeedL", "setspeedR",
            "speedL", "speedR",
            "countsL", "countsR",
            "powerL", "powerR",
            "Kp", "Ki", "Kd"
        ]

    def state( self, d ):
        s = json.dumps( d, separators=(',',':') )
        self.__dict__.update( d )
        self.time = time.time()
        # print( "Target=", self.__dict__, end="\r\n" )
        return s

    def json( self ):
        return json.dumps( self.__dict__, separators=(',',':') )

class RobotController():
    """
    An interface to an Arduino controlled robot.

    Methods:
        __init__(): Construct the interface.
        send(): Send a command to the arduino controller.
        Initialise(): Connect to the arduino and initialise the robot.
        Run(): Tell the robot to move.
        Look(): Point the robot camera in the given direction.
        Track(): Tell the robot to track to the given directions.
        TrackObject(): Tell the robot to track the object at the coordinates.
        RemoteControl(c): Process a keypress as a remote control for robot.
    """

    def __init__( self ):
        """
        Construct an interface object for the Arduino robot.
        """
        self.speed      = 0
        self.power      = 0
        self.direction  = 0
        self.angleX     = 0
        self.angleY     = 0
        self.trackingOn = False
        self.targetstate= RobotState()
        self.robotstate = RobotState()
        self.mqrobot    = MqRobot( self )
        self.datafile   = open( "./robbie.csv", "w", 1 )

        # First line of the data file is the name of the data fields
        print( *self.robotstate.listofkeys(), sep=',',
               file=self.datafile )

    def process_arduino_response( self, s ):
        if (s[0] == "{"):
            self.mqrobot.update( s )
            self.robotstate.update( s )
            # Save the robotstate in the data logging file
            print( *self.robotstate.listofvalues(), sep=',',
                   file=self.datafile )
        else:
            print( "Arduino: ", s, end="\r\n" )

    # Simple method to do range checking
    def _constrain( self, n, minn, maxn ):
        return (minn if n < minn else
                maxn if maxn < n else
                n)

    def send( self, command ):
        """
        Send the supplied command to the Arduino robot controller.

        Arguments:
            command (str): the text of the command to send.
        Returns:
            True on success, False on falure.
        """
        return self.mqrobot.update( command )

    # Connect to and initialise the arduino robot
    def initialise( self ):
        """
        Connect to and initialise the arduino robot
        """
        self.mqrobot.initialise()
        return self.Look( 0, 0 ) and self.Run( 0, 0 )

    def close( self ):
        """
        Shutdown/close the robot.
        """
        print( "Closing down the robot..." )
        self.datafile.close()
        self.mqrobot.close()

    # Tell the robot to move at "speed" in "direction"
    def Run( self, speed, direction=0 ):
        """
        Tell the robot to move at the given speed in the given direction.

        Arguments:
            speed (int): speed at which the robot should move.
            direction: turn left (-1) or right (1), or straight ahead (0)
        """
        self.speed     = self._constrain( speed, -255, 255 )
        self.direction = self._constrain( direction, -0.5, 0.5 )

        difference = direction * 2.0 * self.speed
        left  = speed + 0.5 * difference
        right = speed - 0.5 * difference
        if (left > 255):
            left  = 255
            right = left - difference
        elif (left < -255):
            left  = -255
            right = left + difference
        if (right > 255):
            right = 255
            left  = right + difference
        elif (right < -255):
            right = -255;
            left  = right - difference

        return self.send(
            self.targetstate.state( {"setspeed": [int(round(left)),
                                                  int(round(right))]} )
        )

    # Tell the robot to move at "speed" in "direction"
    def Power( self, power ):
        """
        Set the robot motor power.

        Arguments:
            power (int): power setting for robot motors.
        """
        self.power     = self._constrain( power, -255, 255 )
        return self.send(
            self.targetstate.state( {"power":
                                     [int(round(self.power)),
                                      int(round(self.power))]} )
        )

    # Tell the robot to point camera at "angle"
    def Look( self, angleX, angleY = -1000 ):
        """
        Tell the robot to turn the camera to point in the given direction.

        Arguments:
            angleX: Angle to point the camera (-90.0 to 90.0)
            angleY: Angle to point the camera (-90.0 to 90.0)
        Returns:
            True on success, False on failure.
        """
        self.angleX = self._constrain( angleX, -90, 90 )
        if angleY > -1000:
            self.angleY = self._constrain( angleY, -90, 90 )
        return self.send(
            self.targetstate.state(
                {"head": [ self.angleX, self.angleY]}
            )
        )

    def PID( self, deltaKp, deltaKi, deltaKd ):
        """
        Adjust the PID parameters for the PID control on the motor wheels.
        """
        return self.send(
            self.targetstate.state(
                {"pid": [self.targetstate.pid[0] + deltaKp,
                         self.targetstate.pid[1] + deltaKi,
                         self.targetstate.pid[2] + deltaKd ]}
            )
        )

    # Process any key presses - return False if time to quit
    def RemoteControl( self, c ):
        """
        Interpret a key press (character) as a remote controller for the robot.

        Arguments:
            c: The remote control key.
        """
        # print( ">>  %s" % (c) )
        if c == "q":
            # Quit the program
            self.Run( 0, 0 )
            # self.Look( 0 )
            return None
        elif c == "t":
            # Toggle tracking mode on/off
            self.trackingOn = not self.trackingOn
            return True
        elif c == " ":
            # Stop robot command
            return self.Power( 0 ) and self.Run( 0, 0 )
        elif c == "z":
            # Turn camera/head to left
            return self.Look( self.angleX - 2, self.angleY )
        elif c == "x":
            # Turn camera/head to right
            return self.Look( self.angleX + 2, self.angleY )	
        elif c == "c":
            # Turn camera/head down
            return self.Look( self.angleX, self.angleY - 2 )
        elif c == "d":
            # Turn camera/head up
            return self.Look( self.angleX, self.angleY + 2 )	
        elif c == "s":
            # Look straight ahead
            return self.Look( 0.0, 0.0 )	
        elif c == "g" or c == "\e[A":
            # Up key - Increase robot speed
            return self.Run( self.speed + 2, self.direction )
        elif c == "b" or c == "\e[B":
            # Down key - Decrease robot speed
            return self.Run( self.speed - 2, self.direction )
        elif c == "f":
            # Up key - Increase robot power
            return self.Power( self.power + 10 )
        elif c == "v":
            # Down key - Decrease robot power
            return self.Power( self.power - 10 )
        elif c == "." or c == "\e[C":
            # Right key - Turn robot to the right
            return self.Run( self.speed, self.direction + 0.05 )
        elif c == "," or c == "\e[D":
            # Left key - Turn robot to the left
            return self.Run( self.speed, self.direction - 0.05 )
        elif c == "/":
            # Go straigh ahead
            return self.Run( self.speed, 0.0 )
        elif c == "u":
            # Increment Kp
            return self.PID(  0.1,  0.0,  0.0 )
        elif c == "j":
            # Decrement Kp
            return self.PID( -0.1,  0.0,  0.0 )
        elif c == "i":
            # Increment Ki
            return self.PID(  0.0,  0.1,  0.0 )
        elif c == "k":
            # Decrement Ki
            return self.PID(  0.0, -0.1,  0.0 )
        elif c == "o":
            # Increment Kd
            return self.PID(  0.0,  0.0,  0.1 )
        elif c == "l":
            # Decrement Kd
            return self.PID(  0.0,  0.0, -0.1 )
        return True

    def loop( self ):
        """
        Main execution loop for the robot.
        Reads keypresses and issues commands to control the robot.
        """
        old_settings = termios.tcgetattr( sys.stdin.fileno() )
        try:
            tty.setraw( sys.stdin.fileno() )
            done = False
            while not done:
                print( ">>", end=" " )
                c = sys.stdin.read( 1 )
                if c == "\e":
                    c += sys.stdin.read( 1 )
                    if c[1] == "[":
                        c += sys.stdin.read( 1 )
                print( c, end="\r\n" )
                if self.RemoteControl( c ) is None:
                    done = True
        finally:
            termios.tcsetattr( sys.stdin.fileno(),
                               termios.TCSADRAIN,
                               old_settings )

if __name__ == "__main__":
    robbie = RobotController()

    try:
        robbie.initialise()
        robbie.loop()
        robbie.close()
    except (KeyboardInterrupt, SystemExit):
        robbie.close()
        sys.exit()
