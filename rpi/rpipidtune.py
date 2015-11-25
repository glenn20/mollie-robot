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

import numpy as np
import matplotlib.pyplot as plt
import pandas as pd

class RobotState:
    def __init__( self ):
        self.time       = 0
        self.head       = [0,0]
        self.setspeed   = [0,0]
        self.speed      = [0,0]
        self.counts     = [0,0]
        self.power      = [0,0]
        self.pid        = [0.7, 0.0, 0.0 ]

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

class MqRobot( mqtt.Client ):
    """
    A simple wrapper to send robot status messages to the MQTT broker
    """
    def __init__( self, clientid = None ):
        super( MqRobot, self ).__init__( clientid )
        self.robotstate   = RobotState()
        self.df           = pd.DataFrame( columns=self.robotstate.listofkeys() )
        self.on_connect   = self._on_connect
        self.on_subscribe = self._on_subscribe
        self.on_message   = self._on_message
        self.savedata     = False
        # self.on_log       = self._on_log
        self.connect( "192.168.0.30", 1883, 60 )
        self.loop_start()
        self.send( {"power": [0,0], "setspeed": [0,0]} )
        self.send( {"pid": [0.7, 0.0, 0.0]} )
        plt.ion()

    def _on_connect( self, mqttc, obj, flags, rc ):
        print( "MQTT: Connected: rc: " + str( rc ),
               end="\r\n" )
        self.subscribe( "/mollie-robot/state", 1 )

    def _on_subscribe( self, mqttc, obj, mid, granted_qos ):
        print( "MQTT: Subscribed: " + str( mid ) + " " + str( granted_qos ),
               end="\r\n" )

    def _on_message( self, mqttc, obj, msg ):
        #print( "MQTT: " + msg.topic + " " +
        #       str( msg.qos ) + " " + str( msg.payload ),
        #       end="\r\n" )
        # Update the robot state
        self.robotstate.state( json.loads( msg.payload ) )
        if (self.savedata):
            self.df.loc[len(self.df)] = self.robotstate.listofvalues()

    def _on_log( self, mqttc, obj, level, string ):
        print( "MQTT: " +  string )

    def close( self ):
        self.loop_stop()
        self.disconnect()

    def update( self, state ):
        return self.publish( "/mollie-robot/target", state, qos=0, retain=True )

    def send( self, d ):
        return self.update( json.dumps( d, separators=(',',':') ) )

    def pidcheck( self, K ):
        self.df = pd.DataFrame( columns=self.robotstate.listofkeys() )
        self.savedata = True
        self.send( {"pid": [K[0], K[1], K[2]]} )
        self.send( {"setspeed": [6, 6]} )
        time.sleep( 4 )
        self.send( {"setspeed": [18, 18]} )
        time.sleep( 5 )
        self.send( {"setspeed": [6, 6]} )
        time.sleep( 5 )
        self.savedata = False
        self.send( {"setspeed": [0, 0]} )
        
        self.df.plot(
            x="time",
            y=["setspeedR", "speedR", "powerR"],
            secondary_y=["powerR"]
            #, linestyle='-', marker='o'
        )
        self.df.plot(
            x="time",
            y=["setspeedL", "speedL", "powerL"],
            secondary_y=["powerL"]
            #, style="o-"
        )
        plt.draw()
        # self.df.to_csv( "pidtune-%.1f-%.1f-%.1f.csv" % (K[0], K[1], K[2]) )
        
if __name__ == "__main__":
    
    robbie = MqRobot()
    robbie.pidcheck( [4.0, 0.5, 1.0] )
    robbie.close()
