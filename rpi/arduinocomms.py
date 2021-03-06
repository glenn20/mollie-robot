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
        self.daemon     = True     # This thread may block on serial port

        # Opening the serial port resets the Arduino
        # - wait till it is ready
        s = ""
        while not s == "Robot ready for config\r\n":
            s = self.port.readline()
            print( "Arduino:", s, end="" )
        # Will call the run() method in a new thread...
        self.start()

    def setcallback( self, callback ):
        self.callback = callback

    def run( self ):
        while not self.done:
            s = self.port.readline()
            # print( "Recv: ", s )
            try:
                if self.callback is not None:
                    self.callback( s )
                else:
                    print( "Arduino: ", s )
            except ValueError as msg:
                print( msg, s )


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
        # print( "Send: ", command )
        # Write the command to the arduino
        n = self.serialmonitor.port.write( command + "\n" )
        if (n < len( command + "\n" )):
            print( "ArduinoComms.send(): Write() sent", n, "chars, instead of",
                   len( command + "\n" ) )
            print( "ArduinoComms.send(): command=", command )
        return True

    def close( self ):
        if not self.dummy:
            self.serialmonitor.done = True
            # self.serialmonitor.join()
