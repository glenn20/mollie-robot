"""
Provides a Robot comprising an image capture and processing to track objects.

Image capture is from the Raspberry Pi camera and uses colour processing
to identify objects to be tracked by the robot.

Robot control is through commands passed over the I2C bus to an Arduino Board.
"""

import threading

import cv2
import picamera

#from getch import getch
import arduinocomms
import arduinorobot
import colourtracker
import imageprocessor

class _Getch:
    """Gets a single character from standard input.  Does not echo to the
screen."""
    def __init__(self):
        try:
            self.impl = _GetchWindows()
        except ImportError:
            self.impl = _GetchUnix()

    def __call__(self): return self.impl()


class _GetchUnix:
    def __init__(self):
        import tty, sys

    def __call__(self):
        import sys, tty, termios
        fd = sys.stdin.fileno()
        old_settings = termios.tcgetattr(fd)
        try:
            tty.setraw(sys.stdin.fileno())
            ch = sys.stdin.read(1)
        finally:
            termios.tcsetattr(fd, termios.TCSADRAIN, old_settings)
        return ch

class _GetchWindows:
    def __init__(self):
        import msvcrt

    def __call__(self):
        import msvcrt
        return msvcrt.getch()


getch = _Getch()


class TrackingRobot( threading.Thread ):
    """
    Create an object tracker and remote control for an arduino robot.

    Methods:
    __init__() : Construct a robot from a robot controller and an object tracking module.
    run() : 
    """
    defaultresolution      = ( 320, 240 )
    defaultnumberofthreads = 4

    def __init__( self, robot, tracker,
                  resolution      = None,
                  numberofthreads = None,
                  showpreview     = False ):
        """
        Construct a robot which tracks objects using the RPI camera.

        Arguments:
            robot (ArduinoRobot): Interface to the arduino-controlled robot
            tracker (ColourTracker): The object tracking class.
            resolution (x,y): Resolution for image capture (RPI camera)
            numberofthreads: Number of threads for image processing
        """
        super( TrackingRobot, self ).__init__()
        self.robot           = robot
        self.tracker         = tracker
        self.resolution      = (resolution if resolution is not None
                                else self.defaultresolution)
        self.numberofthreads = (numberofthreads if numberofthreads is not None
                                else self.defaultnumberofthreads)
        self.showpreview     = showpreview
        self.start()    # Run the "run()" method in a new thread

    # Process any pending key presses in the opencv window
    # Process these as remote controls for the robot
    def _handlekeypresses( self ):
        for i in range(5):
            c = cv2.waitKey( 50 )
            if c < 0:        # No keys ready
                return True
            c = chr( c & 255 )  # Bugfix - this is needed for opencv on 64bit linux
            if self.robot.RemoteControl( c ) is None:
                return False   # Quit key ("q") was pressed
        return True

    # The function to process each captured image
    # Process each image to identify location of object
    # Send the location of the object to the arduino
    # Process any key presses as remotecontrol for the robot
    def doObjectTracking( self, stream ):
        """
        Process captured images and locate object for tracking.

        Arguments:
            stream (BytesIO): stream containing captured image data
        Returns:
            Return False if we detect a shutdown request
        """
        # Get the position of the object being tracked
        posX, posY, area = self.tracker.Track( stream )
        # Send the coordinates to the robot
        self.robot.TrackObject( posX, posY, area )
        # Process any pending key presses
        return self._handlekeypresses()

    # Setup the camera and run the image capture process
    def run( self ):
        """
        Run the image capture and processing and robot control loop.

        Setups up and executes the
        multi-threaded image capture and processing.
        """
        with picamera.PiCamera() as camera:
            camera.preview_fullscreen = False
            camera.preview_window     = (100, 100,
                                         self.resolution[0],
                                         self.resolution[1] )
            camera.resolution         = self.resolution
            camera.framerate          = 10
            # camera.exposure_mode    = 'off'
            camera.ISO                = 800
            camera.image_effect       = 'blur'
            camera.awb_mode           = 'off' # 'fluorescent'
            camera.awb_gains          = (1.2,1.2)
            if (self.showpreview):
				camera.start_preview()
            # time.sleep(2)
            # Setup the multi-threaded image processing factory
            with imageprocessor.ProcessorManager(
                numberofthreads       = self.numberofthreads,
                processingfunction    = self.doObjectTracking
                ) as processor:
                # Start the PiCamera capture_sequence
                camera.capture_sequence( processor.streamgenerator(),
                                         use_video_port=True )

    def loop( self ):
        """
        Main execution loop for the robot.
        Reads keypresses and issues commands to control the robot.
        """
        while True:
            print( "Call getch()..." )
            c = getch()
            print( ">>  %s\r" % (c) )
            if self.robot.RemoteControl( c ) is None:
                return False   # Quit key ("q") was pressed

    def close( self ):
        """
        Close down the robot controller and object tracking components. 
        """
        self.tracker.close()
        self.robot.close()

# Local Variables:
# python-indent: 4
# tab-width: 4
# indent-tabs-mode: nil
# End: 
