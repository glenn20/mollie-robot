# Construct robots with a comms object
# Just requires a "send" method to send commands to the arduino
class ArduinoRobot():
    def __init__( self, arduinoComms ):
        self.speed      = 0
        self.direction  = 0
        self.angle      = 0
        self.trackingOn = False
        self.arduino    = arduinoComms

    # Simple method to do range checking
    def constrain( self, n, minn, maxn ):
        if n < minn:
            return minn
        elif n > maxn:
            return maxn
        else:
            return n

    def send( self, command ):
        return self.arduino.send( command )

    # Tell the robot to move at "speed" in "direction"
    def Go( self, speed, direction=0 ):
        self.speed     = self.constrain( speed, -255, 255 )
        self.direction = self.constrain( direction, -0.1, 0.1 )
        return self.send( "run %d %d"
                          % (int(round(self.speed)),
                             int(round(self.direction * 1000))) )

    # Tell the robot to point camera at "angle"
    def Servo( self, angle ):
        self.angle = self.constrain( angle, -90, 90 )
        return self.send( "servo %d"
                          % (angle) ) 

    # Tell the robot to track to the given angles
    def Track( self, x, y ):
        return self.send( "track %d %d"
                          % (x, y) ) 

    def TrackObject( self, posX, posY, area ):
        # If the image is big enough - track it!!!
        if area > 50:
            if self.trackingOn == True:
                # Tell the robot to look here
                return self.Track( posX, posY )
        elif self.trackingOn == True:
            # If we lost the object - tell the robot to go straight
            if self.direction != 0 and self.speed != 0:
                return self.Go( self.speed, 0 )
    
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
            return None
        elif c == "t":
            # Toggle tracking mode on/off
            self.trackingOn = not self.trackingOn
            return True
        elif c == " ":
            # Stop robot command
            return self.Go( 0, 0 )
        elif c == "z":
            # Turn camera/head to left
            return self.Servo( self.angle - 2 )
        elif c == "x":
            # Turn camera/head to right
            return self.Servo( self.angle + 2 )	
        elif c == "R":
            # Up key - Increase robot speed
            return self.Go( self.speed + 10, self.direction )
        elif c == "T":
            # Down key - Decrease robot speed
            return self.Go( self.speed - 10, self.direction )
        elif c == "S":
            # Right key - Turn robot to the right
            return self.Go( self.speed, self.direction + 0.02 )
        elif c == "Q":
            # Left key - Turn robot to the left
            return self.Go( self.speed, self.direction - 0.02 )
        return True

    def close():
        pass
