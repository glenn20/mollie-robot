import smbus
import time
import io

class ArduinoComms():
    def __init__( self, I2Cbusnumber = 0, I2Caddress = 0x01, dummy = False ):
        # for RPI version 1, use "busnum = 0"
        # for RPI version 2, use "busnum = 1"
        self.busnum  = I2Cbusnumber
        # The I2C address of the arduino - setup in the arduinoRobot Program
        self.address = I2Caddress
        self.dummy   = dummy
        self.bus     = smbus.SMBus( self.busnum ) if not dummy else None

    # Write one character to the I2C bus
    def writeCharacter( self, c ):
        # Retry four times in case we get an IOError on bus.write_byte()
        for i in range(4):
            try:
                self.bus.write_byte( self.address, ord(c) )
                return True
            except IOError:
                print( "writeCommand(): IOError: trying again" )
        return False

    # Read one character from the I2C bus
    def readCharacter( self ):
        # Retry 4 times in case we get an IOError on bus.read_byte()
        for i in range(4):
            try:
                c = self.bus.read_byte( self.address )
                return c
            except IOError:
                print( "readCommand(): IOError: trying again" )
        return -1

    # Write a whole line of text to the I2C bus and end with a newline char
    def writeline( self, line ):
        for c in line:
            self.writeCharacter( c )
        # Add an end-of-line character at the end
        self.writeCharacter( '\n' )
        return -1

    # Read back the status after writing a command to the Arduino
    def readStatus( self ):
        number = self.readCharacter()
        return number

    # Write a command to the Arduino over I2C and check the status
    # Wait for the status response to be non-zero
    def send( self, command ):
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

