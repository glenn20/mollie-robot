#!/usr/bin/env python

"""
An interface to manage an Arduino controlled robot.

Classes:
    ArduinoRobot: The manager of the Arduino robot.
"""

import argparse
import sys

parser = argparse.ArgumentParser(
    description='Run the RPI-arduino proxy control program.'
    )
parser.add_argument(
    "--verbose", action="store_true",
    help="Print diagnostic output"
    )
args = parser.parse_args()


from __future__ import print_function
import itertools
import time
import json
import jsmin
import paho.mqtt.client

import arduinocomms

class MqRobot( paho.mqtt.client.Client ):
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
        self.subscribe( "/mollie-robot/target", 1 )

    def _on_subscribe( self, mqttc, obj, mid, granted_qos ):
        print( "MQTT: Subscribed: " + str( mid ) + " " + str( granted_qos ) )

    def _on_message( self, mqttc, obj, msg ):
        if args.verbose:
                print( "MQTT: " + msg.topic + " " +
                       str( msg.qos ) + " " + str( msg.payload ) )
        try:
            d = {"target": json.loads( msg.payload )}
            s = self.robot.targetstate.state( d )
            self.robot.send( s )
        except ValueError as e:
            print( e, msg.payload )

    def _on_log( self, mqttc, obj, level, string ):
        print( "MQTT: " +  string )

    def initialise( self ):
        self.connect( "127.0.0.1", 1883, 60 )
        self.loop_start()

    def close( self ):
        self.loop_stop()
        self.disconnect()

    def update( self, state ):
        self.publish( "/mollie-robot/state", state, qos=0, retain=True )


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
        if args.verbose:
            print( "Line =", s )
        d = json.loads( s )
        self.__dict__.update( d )
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
        return s

    def json( self ):
        return json.dumps( self.__dict__, separators=(',',':') )

# Construct robots with a comms object
# Just requires a "send" method to send commands to the arduino
class ArduinoProxy():
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

    def __init__( self, arduinoComms ):
        """
        Construct an interface object for the Arduino robot.

        Arguments:
            arduinoComms (ArduinoComms): I2C bus comms interface
        """
        self.speed      = 0
        self.power      = 0
        self.direction  = 0
        self.angleX     = 0
        self.angleY     = 0
        self.trackingOn = False
        self.arduino    = arduinoComms
        self.targetstate= RobotState()
        self.robotstate = RobotState()
        self.mqrobot    = MqRobot( self )

        self.arduino.setcallback( self.process_arduino_response )

    def process_arduino_response( self, s ):
        if (s[0] == "{"):
            self.robotstate.update( s )
            self.mqrobot.update( s )
        else:
            print( "Arduino: ", s, end="" )

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
        return self.arduino.send( command )

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
        self.arduino.close()
        self.mqrobot.close()

    def loop( self ):
        while True:
            time.sleep( 1 )


if __name__ == "__main__":
    robbie = ArduinoProxy(
        arduinocomms.ArduinoComms(
            device       = "/dev/ttyS99",
            baudrate     = 115200,
            dummy        = False
        )
    )

    robbie.initialise()

    # Load and send configuration to the robot.
    with open( 'config.json', 'r' ) as f:
        d = json.loads( jsmin.jsmin( f.read() ) )

    # Break up the config dictionary into parts
    for key, value in d["config"].iteritems():
        for key2, value2 in value.iteritems():
            s = json.dumps( {"config": {key: {key2: value2}}},
                            separators=(',',':') )
            if args.verbose:
                print( "Config: ", s )
            robbie.send( s )
            # time.sleep( 1 )

    try:
        robbie.loop()
        robbie.close()
    except (KeyboardInterrupt, SystemExit):
        robbie.close()
        sys.exit()
