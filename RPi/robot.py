from rpiarduinocomms import sendArduino

def clamp(n, minn, maxn):
    if n < minn:
        return minn
    elif n > maxn:
        return maxn
    else:
        return n

class Robot():
    def __init__( self ):
        self.speed      = 0
        self.direction  = 0
        self.angle      = 0
        self.trackingOn = False

    # Tell the robot to move at "speed" in "direction"
    def Go( self, speed, direction=0 ):
        self.speed     = clamp( speed, -255, 255 )
        self.direction = clamp( direction, -0.1, 0.1 )
        sendArduino( "run %d %d"
                     % (int(round(self.speed)),
                        int(round(self.direction * 1000))) )
        return True

    # Tell the robot to point camera at "angle"
    def Servo( self, angle ):
        self.angle = clamp( angle, -90, 90 )
        sendArduino( "servo %d"
                     % (angle) ) 
        return True

    # Tell the robot to track to the given angles
    def Track( self, x, y ):
        sendArduino( "track %d %d"
                     % (x, y) ) 
        return True

    def TrackObject( self, posX, posY, area ):
        # If the image is big enough - track it!!!
        if area > 50:
            if self.trackingOn == True:
                # Tell the robot to look here
                self.Track( posX, posY )
        elif self.trackingOn == True:
            # If we lost the object - tell the robot to go straight
            if self.direction != 0 and self.speed != 0:
                self.Go( self.speed, 0 )
    
    # Process any key presses - return False if time to quit
    def RemoteControl( self, c ):
        if c < 0:
            return True
        c = chr( c & 255 )  # Bugfix - this is needed for opencv on 64bit linux
        print( ">>  %s" % (c) )
        if c == "q":
            # Quit the program
            self.Go( 0, 0 )
            # self.Servo( 0 )
            return False
        elif c == "t":
            # Toggle tracking mode on/off
            self.trackingOn = not self.trackingOn
        elif c == " ":
            # Stop robot command
            self.Go( 0, 0 )
        elif c == "z":
            # Turn camera/head to left
            self.Servo( self.angle - 2 )
        elif c == "x":
            # Turn camera/head to right
            self.Servo( self.angle + 2 )	
        elif c == "R":
            # Up key - Increase robot speed
            self.Go( self.speed + 10, self.direction )
        elif c == "T":
            # Down key - Decrease robot speed
            self.Go( self.speed - 10, self.direction )
        elif c == "S":
            # Right key - Turn robot to the right
            self.Go( self.speed, self.direction + 0.02 )
        elif c == "Q":
            # Left key - Turn robot to the left
            self.Go( self.speed, self.direction - 0.02 )
        return True
