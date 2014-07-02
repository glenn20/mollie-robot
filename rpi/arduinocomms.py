"""
Arduino communications interface - over I2C bus.
"""

import smbus
import time
import io

class ArduinoComms():
    """
    Communications interface from RPI to Arduino over I2C bus.

    Methods:
        __init__(): Construct a comms controller.
        send(): Send a command to the Arduino robot and check status byte.
    """
    def __init__( self, I2Cbusnumber = 0, I2Caddress = 0x01, dummy = False ):
        """
        Construct an interface object for comms with Arduino over I2C bus.

        Arguments:
            I2Cbusnumber (int): The number of the I2C bus to use on the RPI.
            I2Caddress (int): Address of the Arduino on the I2C bus.
            dummy (= False): Flag to fake sending comms to Arduino (used for testing)
        
        for RPI version 1, use I2Cbusnumber = 0
        for RPI version 2, use I2Cbusnumber = 1
        """
        self.busnum  = I2Cbusnumber
        # The I2C address of the arduino - setup in the arduinoRobot Program
        self.address = I2Caddress
        self.dummy   = dummy
        self.bus     = smbus.SMBus( self.busnum ) if not dummy else None

    # Write one character to the I2C bus
    def _writecharacter( self, c ):
        """
        Write one character to the Arduino controller over I2C bus.

        Arguments:
            c (str): A single character to write.
        Returns:
            True on success and False on failure.
        """
        # Retry four times in case we get an IOError on bus.write_byte()
        for i in range(4):
            try:
                self.bus.write_byte( self.address, ord(c) )
                return True
            except IOError:
                print( "writeCommand(): IOError: trying again" )
        return False

    # Read one character from the I2C bus
    def _readcharacter( self ):
        """
        Read one character from the Arduino controller over I2C bus.

        Returns:
            Character read from bus if successful, else return -1.
        """
        # Retry 4 times in case we get an IOError on bus.read_byte()
        for i in range(4):
            try:
                c = self.bus.read_byte( self.address )
                return c
            except IOError:
                print( "readCommand(): IOError: trying again" )
        return -1

    # Write a whole line of text to the I2C bus and end with a newline char
    def _writeline( self, line ):
        """
        Write a line of text to the Arduino over I2C bus.

        Will send a newline character after transmitting the string.

        Arguments:
            line (str): Line of text to transmit to the Arduino.
        """
        for c in line:
            self._writecharacter( c )
        # Add an end-of-line character at the end
        self._writecharacter( '\n' )
        return -1

    # Read back the status after writing a command to the Arduino
    def _readStatus( self ):
        """
        Read a status byte back from the Arduino.

        This is used after sending command to arduino to confirm receipt.

        Returns:
            Status byte if successful, else -1.
        """
        number = self._readcharacter()
        return number

    # Write a command to the Arduino over I2C and check the status
    # Wait for the status response to be non-zero
    def send( self, command ):
        """
        Write a command to the Arduino over the I2C bus.

        Writes the command and waits for Arduino to acknowledge receipt.

        Arguments:
            command (str): The command text to be sent to the Arduino.
        Returns:
            True on success and False on failure.
        """
        if self.dummy:     # For testing - do dummy writes
            print( command )   # Diagnostic
            return True
        # Write the command to the arduino
        self.writeline( command )
        i = 0
        # Check the status from the arduino - wait for it to be non-zero
        while status == 0 and i < 20:
            status = self.readStatus()
            i=i+1
        if status == 0:
            # Timed out waiting for a non-zero status response
            print( "ArduinoI2CComms.send():"
                   "I2C timeout - status = %d" % (status) )
            return False
        elif status != 1:
            # We expect the status to be 1 when all is OK
            # But, often it is not (eg. 9) - don't know why
            # Seems to work OK anyway...
            # print( "ArduinoI2CComms.send():"
            #        "Unexpected I2C status response = %d" % (status) )
            pass
        return True

