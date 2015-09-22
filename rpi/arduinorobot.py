"""
An interface to manage an Arduino controlled robot.

Classes:
    ArduinoRobot: The manager of the Arduino robot.
"""

# Construct robots with a comms object
# Just requires a "send" method to send commands to the arduino
class ArduinoRobot():
    """
    An interface to an Arduino controlled robot.

    Methods:
        __init__(): Construct the interface.
        send(): Send a command to the arduino controller.
        Initialise(): Connect to the arduino and initialise the robot.
        Run(): Tell the robot to move.
        Look(): Point the robot camera in the given direction.
        Look(): Point the robot camera in the given direction.
        Track(): Tell the robot to track to the given directions.
        TrackObject(): Tell the robot to track the object at the coordinates.
        RemoteControl(c): Process a keypress as a remote control for robot.
    """

    def __init__( self, arduinoComms ):
        """
        Construct an interface object for the Arduino robot.

        Arguments:
            arduinoComms (ArduinoComms): I2C bus comms interface
        """
        self.speed      = 0
        self.power      = 0
        self.direction  = 0
        self.angleX     = 0
        self.angleY     = 0
        self.trackingOn = False
        self.arduino    = arduinoComms

    # Simple method to do range checking
    def _constrain( self, n, minn, maxn ):
        return (n if minn <= n <= maxn
                else (minn if n < minn
                      else maxn))

    def send( self, command ):
        """
        Send the supplied command to the Arduino robot controller.

        Arguments:
            command (str): the text of the command to send.
        Returns:
            True on success, False on falure.
        """
        return self.arduino.send( command )

    # Connect to and initialise the arduino robot
    def Initialise( self ):
        """
        Connect to and initialise the arduino robot
        """
        return self.Look( 0, 0 ) and self.Run( 0, 0 )

    # Tell the robot to move at "speed" in "direction"
    def Run( self, speed, direction=0 ):
        """
        Tell the robot to move at the given speed in the given direction.

        Arguments:
            speed (int): speed at which the robot should move.
            direction: turn left (-1) or right (1), or straight ahead (0)
        """
        self.speed     = self._constrain( speed, -255, 255 )
        self.direction = self._constrain( direction, -0.1, 0.1 )
        return self.send( "run %d %d"
                          % (int(round(self.speed)),
                             int(round(self.direction * 1000))) )

    # Tell the robot to move at "speed" in "direction"
    def Power( self, power ):
        """
        Set the robot motor power.

        Arguments:
            power (int): power setting for robot motors.
        """
        self.power     = self._constrain( power, -255, 255 )
        return self.send( "setpower %d %d"
                          % (int(round(self.power)),
                             int(round(self.power))) )

    # Tell the robot to point camera at "angle"
    def Look( self, angleX, angleY = -1000 ):
        """
        Tell the robot to turn the camera to point in the given direction.

        Arguments:
            angleX: Angle to point the camera (-90.0 to 90.0)
            angleY: Angle to point the camera (-90.0 to 90.0)
        Returns:
            True on success, False on failure.
        """
        self.angleX = self._constrain( angleX, -90, 90 )
        if angleY > -1000:
            self.angleY = self._constrain( angleY, -90, 90 )
        return self.send( "look %d %d"
                          % (self.angleX, self.angleY) )

    # Tell the robot to track to the given angles
    def Track( self, x, y ):
        """
        Tell the robot controller to track the object in the given direction.

        Arguments:
            x: Horizontal angle of the object to track.
            y: Vertical angle of the object to track.
        """
        return self.send( "track %d %d"
                          % (x, y) )

    def TrackObject( self, posX, posY, area ):
        """
        Tell the robot controller to track the object if tracking is enabled.

        Also checks the identified object is large enough.

        Arguments:
            posX: The horizontal angle of the object.
            posY: The vertical angle of the object.
            area: The area of the identified object.
        """
        # If the image is big enough - track it!!!
        posX *= 20.0 / 115.0    # Calibrate - convert camera pixels to degrees
        posY *= 20.0 / 115.0
        # print( "track %d %d\r\n" % (posX, posY) )
        if area > 50:
            if self.trackingOn == True:
                # Tell the robot to look here
                return self.Track( posX, posY )
        elif self.trackingOn == True:
            # If we lost the object - tell the robot to go straight
            if self.direction != 0 and self.speed != 0:
                return self.Run( self.speed, 0 )
    
    # Process any key presses - return False if time to quit
    def RemoteControl( self, c ):
        """
        Interpret a key press (character) as a remote controller for the robot.

        Arguments:
            c: The remote control key.
        """
        # print( ">>  %s" % (c) )
        if c == "q":
            # Quit the program
            self.Run( 0, 0 )
            # self.Look( 0 )
            return None
        elif c == "t":
            # Toggle tracking mode on/off
            self.trackingOn = not self.trackingOn
            return True
        elif c == " ":
            # Stop robot command
            return self.Power( 0 )
        elif c == "z":
            # Turn camera/head to left
            return self.Look( self.angleX - 2, self.angleY )
        elif c == "x":
            # Turn camera/head to right
            return self.Look( self.angleX + 2, self.angleY )	
        elif c == "c":
            # Turn camera/head down
            return self.Look( self.angleX, self.angleY - 2 )
        elif c == "d":
            # Turn camera/head up
            return self.Look( self.angleX, self.angleY + 2 )	
        elif c == "s":
            # Look straight ahead
            return self.Look( 0.0, 0.0 )	
        elif c == "R" or c == "g":
            # Up key - Increase robot speed
            return self.Run( self.speed + 2, self.direction )
        elif c == "T" or c == "b":
            # Down key - Decrease robot speed
            return self.Run( self.speed - 2, self.direction )
        elif c == "f":
            # Up key - Increase robot speed
            return self.Power( self.power + 10 )
        elif c == "v":
            # Down key - Decrease robot speed
            return self.Power( self.power - 10 )
        elif c == "S":
            # Right key - Turn robot to the right
            return self.Run( self.speed, self.direction + 0.02 )
        elif c == "Q":
            # Left key - Turn robot to the left
            return self.Run( self.speed, self.direction - 0.02 )
        return True

    def close( self ):
        """
        Shutdown/close the robot.
        """
        print( "Closing down the robot..." )
        self.arduinocomms.serialmonitor.done = True
