"""
Arduino communications interface - over Serial bus.
"""

from __future__ import print_function
import serial
import threading

class SerialMonitor( threading.Thread ):
    """
    A class to monitor the serial port in a thread
    """
    def __init__( self, device, baudrate ):
        super( SerialMonitor, self ).__init__()
        self.device     = device
        self.port       = serial.Serial( self.device, baudrate=baudrate, timeout=None )
        self.callback   = None
        self.done       = False

        # Opening the serial port resets the Arduino
        # - wait till it is ready
        s = ""
        while not s == "Robot ready\r\n":
            s = self.port.readline()
            print ( "Arduino:", s )
        print ( "Arduino:", s )
        # Will call the run() method in a new thread...
        self.start()

    def setcallback( self, callback ):
        self.callback = callback

    def run( self ):
        while not self.done:
            s = self.port.readline()
            try:
                if self.callback is not None:
                    self.callback( s )
                else:
                    print( "Arduino: ", s, "\r\n" )
            except ValueError as msg:
                print( msg, s, end="\r\n" )


class ArduinoComms():
    """
    Communications interface from RPI to Arduino over Serial bus.

    Methods:
        __init__(): Construct a comms controller.
        send():  Send a command to the Arduino robot.
        close(): Close down the serial port monitor thread
    """
    def __init__( self, device="/dev/ttyS99", baudrate=115200, dummy = False ):
        """
        Construct an interface object for comms with Arduino over Serial bus.

        Arguments:
            dummy (= False): Flag to fake sending comms to Arduino (used for testing)
        
        """
        self.dummy = dummy
        if not self.dummy:
            self.serialmonitor = SerialMonitor( device, baudrate )

    def setcallback( self, callback ):
        if not self.dummy:
            self.serialmonitor.setcallback( callback )

    # Write a command to the Arduino over serial
    def send( self, command ):
        """
        Write a command to the Arduino over the I2C bus.

        Arguments:
            command (str): The command text to be sent to the Arduino.
        Returns:
            True on success and False on failure.
        """
        if self.dummy:     # For testing - do dummy writes
            print( command )   # Diagnostic
            return True
        # Write the command to the arduino
        self.serialmonitor.port.write( command + "\n" )
        return True

    def close( self ):
        if not self.dummy:
            self.serialmonitor.done = True
            # self.serialmonitor.join()
