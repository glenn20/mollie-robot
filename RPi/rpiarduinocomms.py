import smbus
import time

# for RPI version 1, use "bus = smbus.SMBus(0)"
# for RPI version 2, use "bus = smbus.SMBus(1)"
bus = smbus.SMBus(1)

# This is the address we setup in the Arduino Program
address = 0x04

def writeCharacter( c ):
	for i in range(4):
		try:
			bus.write_byte(address, ord(c))
			return True
		except IOError:
			print("writeCommand(): IOError: trying again")
	return False

def readCharacter():
	for i in range(4):
		try:
			number = bus.read_byte(address)
			return number
		except IOError:
			print("readCommand(): IOError: trying again")
	return -1

def writeCommand(command):
	for c in command:
		writeCharacter( c )
	writeCharacter( '\n' )
	return -1

def readStatus():
	number = readCharacter()
	return number

def sendArduino( command ):
	print( command )
	writeCommand(command)
	status = 0
	i = 0
	while status == 0:
		status = readStatus()
		i=i+1
#	if status == 1:
#		print "OK"
#	else:
#		print "Status = ", status

def Go(speed, direction=0):
	#=0 means that you dont have to include a direction
	if speed < -255 or speed > 255:
		return False
	if direction < -1.0 or direction > 1.0:
		return False
	sendArduino("run " + str(int(round(speed))) + " " + str(int(round(direction * 1000))))
	return True

def Servo(angle):
	if angle < -90 or angle > 90:
		return False
	sendArduino("servo " + str(angle)) 
	return True

def Track(x, y):
	sendArduino("track " + str(x) + " " + str(y)) 
	return True
