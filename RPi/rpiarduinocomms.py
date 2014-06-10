import smbus
import time

# This is the address we setup in the Arduino Program
busnum = 1
address = 0x04

# for RPI version 1, use "bus = smbus.SMBus( 0 )"
# for RPI version 2, use "bus = smbus.SMBus( 1 )"
#bus = smbus.SMBus( busnum )

# Write one character to the I2C bus
def writeCharacter( c ):
    # Retry four times in case we get an IOError on bus.write_byte()
    for i in range(4):
        try:
            bus.write_byte( address, ord(c) )
            return True
        except IOError:
            print( "writeCommand(): IOError: trying again" )
    return False

# Read one character from the I2C bus
def readCharacter():
    # Retry 4 times in case we get an IOError on bus.read_byte()
    for i in range(4):
        try:
            c = bus.read_byte( address )
            return c
        except IOError:
            print( "readCommand(): IOError: trying again" )
    return -1

# Write a whole line of text to the I2C bus
def writeCommand( command ):
    for c in command:
        writeCharacter( c )
    # Add an end-of-line character at the end
    writeCharacter( '\n' )
    return -1

# Read back the status after writing a command to the Arduino
def readStatus():
    number = readCharacter()
    return number

# Write a command to the Arduino over I2C and check the status
# Wait for the status response to be non-zero
def sendArduino( command ):
    print( command )
    return 1
    writeCommand( command )
    i = 0
    status = readStatus()
    # If the status is still 0, keep trying up to 20 times
    while status == 0 and i < 20:
        status = readStatus()
        i=i+1
    if status != 0:
        print( "sendArduino(): I2C timeout - status = %d" % (status) )
    return status
    # We expect the status to be 1 when all is OK
    # But, often it is not (eg. 9) - don't know why
#   if status == 1:
#       print "OK"
#   else:
#       print "Status = ", status

