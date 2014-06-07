# from getch import getch
from rpiarduinocomms import Go, Servo, Track
from colourtracking import SetupColourTracking, ColourTracking
import cv2.cv as cv

def main():
	speed = 0
	direction = 0
	angle = 0
	trackingOn = False
	# Servo(0)
	SetupColourTracking()
	Go(speed, direction)

	while True:
		track = ColourTracking()
		if track != False:
			posX, posY, area = track
			print( "Track[" + str(trackingOn) + "] " + str(posX) + " " + str(posY) + " " + str(area) )
			if trackingOn == True:
				Track(posX, posY)
		elif trackingOn == True:
			if (direction != 0):
				direction = 0
				Go( speed, direction )

		c = cv.WaitKey(50)
		if c < 0:
			continue
		c = chr(c & 255)
		print(">>", c)
		if c == "q":
		# Quit the program
			Go(0,0)
			#Servo(0)
			return
		elif c == "t":
			trackingOn = not trackingOn
		elif c == " ":
			speed = 0
			direction = 0
			Go(speed,direction)
		elif c == "z":
			angle = angle - 2
			if angle < -90:
				angle = -90
			Servo(angle)
		elif c == "x":
			angle = angle + 2
			if angle > 90:
				angle = 90
			Servo(angle)	
		elif c == "R":
			#Up key
			speed = speed+10
			if speed > 255:
				speed = 255
			Go(speed, direction)
		elif c == "T":
			#Down key
			speed = speed-10
			if speed < -255:
				speed = -255
			Go(speed,direction) 
		elif c == "S":
			#Right key
			direction = direction+0.02
			if direction > 0.1:
				direction = 0.1
			Go(speed, direction)
		elif c == "Q":
			#Left key
			direction = direction-0.02
			if direction < -0.1:
				direction = -0.1
			Go(speed, direction)

	return;

if __name__ == "__main__":
    main()
