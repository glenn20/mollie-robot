"""
An interface to manage an Arduino controlled robot.

Classes:
    ArduinoRobot: The manager of the Arduino robot.
"""

from __future__ import print_function
import itertools
import time
import json

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
        print( "MQTT: Connected: rc: " + str( rc ) )
        self.subscribe( "/mollie-robot/state", 1 )

    def _on_subscribe( self, mqttc, obj, mid, granted_qos ):
        print( "MQTT: Subscribed: " + str( mid ) + " " + str( granted_qos ) )

    def _on_message( self, mqttc, obj, msg ):
        print( "MQTT: " + msg.topic + " " +
               str( msg.qos ) + " " + str( msg.payload ) )
        self.robot.robotstate.state( json.loads( msg.payload ) )

    def _on_log( self, mqttc, obj, level, string ):
        print( "MQTT: " +  string )

    def initialise( self ):
        self.connect( "127.0.0.1", 1883, 60 )
        self.loop_start()

    def close( self ):
        self.loop_stop()
        self.disconnect()

    def update( self, state ):
        self.publish( "/mollie-robot/target", state, qos=0, retain=True )


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
        print( "Line =", s )
        d = json.loads( s )
        self.__dict__.update( d )
        # print( "Robot =", self.__dict__ )
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
        # print( "Target=", self.__dict__ )
        return s

    def json( self ):
        return json.dumps( self.__dict__, separators=(',',':') )

# Just requires a "send" method to send commands to the arduino
class ArduinoRobot():
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

    def close( self ):
        """
        Shutdown/close the robot.
        """
        print( "Closing down the robot..." )
        self.mqrobot.close()

    # Tell the robot to track to the given angles
    def Track( self, x, y ):
        """
        Tell the robot controller to track the object in the given direction.

        Arguments:
            x: Horizontal angle of the object to track.
            y: Vertical angle of the object to track.
        """
        return self.send( "{track: [%d, %d]}"
                          % (x, y) )

    def TrackObject( self, posX, posY, area ):
        """
        Tell the robot controller to track the object if tracking is enabled.

        Also checks the identified object is large enough.

        Arguments:
            posX: The horizontal angle of the object.
            posY: The vertical angle of the object.
            area: The area of the identified object.
        """
        # If the image is big enough - track it!!!
        # posX *= 20.0 / 115.0    # Calibrate - convert camera pixels to degrees
        # posY *= 20.0 / 115.0
        # print( "track %d %d" % (posX, posY) )
        if area > 50:
            if self.trackingOn == True:
                # Tell the robot to look here
                return self.Track( posX, posY )
        elif self.trackingOn == True:
            # If we lost the object - tell the robot to go straight
            if self.direction != 0 and self.speed != 0:
                return self.Run( self.speed, 0 )
