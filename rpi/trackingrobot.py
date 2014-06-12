import io
import cv2
import numpy as np
import picamera

# from getch import getch
import arduinocomms
import arduinorobot
import colourtracker
import imageprocessor


class TrackingRobot:
    defaultresolution      = ( 320, 240 )
    defaultnumberofthreads = 4

    def __init__( self, robot, tracker,
                  resolution      = None,
                  numberofthreads = None ):
        self.robot           = robot
        self.tracker         = tracker
        self.resolution      = resolution \
            if resolution is None \
            else self.defaultresolution
        self.numberofthreads = numberofthreads \
            if numberofthreads is None \
            else self.defaultnumberofthreads

    # The function to process each captured image
    # Process each image to identify location of object
    # Send the location of the object to the arduino
    # Process any key presses as remotecontrol for the robot
    def doObjectTracking( self, stream ):
        # Get the position of the object being tracked
        posX, posY, area = self.tracker.Track( stream )
        # Send the coordinates to the robot
        self.robot.TrackObject( posX, posY, area )
        # Process any pending key presses (up to 5 at a time)
        for i in range(5):
            c = cv2.waitKey( 50 )
            if (c < 0):  # No keys ready
                return True
            if self.robot.RemoteControl( c ) is None:
                return False
        return True

    # Setup the camera and run the image capture process
    def run( self ):
        with picamera.PiCamera() as camera:
            camera.preview_fullscreen = False
            camera.preview_window     = (100, 100,
                                         self.resolution[0],
                                         self.resolution[1] )
            camera.resolution         = self.resolution
            camera.framerate          = 10
            # camera.exposure_mode     = 'off'
            camera.ISO                = 800
            camera.image_effect       = 'blur'
            camera.awb_mode           = 'off' # 'fluorescent'
            camera.awb_gains          = (1.2,1.2)
            # camera.start_preview()
            # time.sleep(2)
            # Setup the multi-threaded image processing factory
            with imageprocessor.ProcessorManager(
                numberofthreads      = self.numberofthreads,
                processingfunction   = self.doObjectTracking
                ) as processors:
                camera.capture_sequence(
                    processors.streams(),
                    use_video_port=True )

    def close( self ):
        self.tracker.close()
        self.robot.close()

# Local Variables:
# python-indent: 4
# tab-width: 4
# indent-tabs-mode: nil
# End: 
