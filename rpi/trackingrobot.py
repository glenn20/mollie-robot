"""
Provides a Robot comprising an image capture and processing to track objects.

Image capture is from the Raspberry Pi camera and uses colour processing
to identify objects to be tracked by the robot.

Robot control is through commands passed over the I2C bus to an Arduino Board.
"""

from __future__ import print_function

import threading
import Queue
import time
import sys, tty, termios

import cv2

#from getch import getch
import arduinocomms
import arduinorobot
import colourtracker
import cameracapture
import workflow

class TrackingRobot():
    """
    Create an object tracker and remote control for an arduino robot.

    Methods:
    __init__() : Create an object tracking arduino controller robot.
    run() : 
    """
    def __init__( self, robot, tracker, camera,
                  showimages = False, showpreview = False ):
        """
        Construct a robot which tracks objects using the RPI camera.

        Arguments:
            robot   (ArduinoRobot): Interface to the arduino-controlled robot
            tracker (ColourTracker): The object tracking class.
            camera: (PiCamera.picamera): Raspberry Pi Camera instance
        """
        self.robot           = robot
        self.tracker         = tracker
        self.camera          = camera
        self.showimages      = showimages
        self.showpreview     = showpreview

        self.done            = False
        self.cameraqueue     = Queue.Queue()
        self.processingqueue = Queue.Queue()
        self.displayqueue    = Queue.Queue( 1 )
        self.workflow        = None
        self.capture         = None

    def displayimage( self, image ):
        self.tracker.Showimage( image )
        return image

    # The function to process each captured image
    # Process each image to identify location of object
    # Send the location of the object to the arduino
    def imagetracking( self, image, otherqueues ):
        """
        Process captured images and locate object for tracking.

        Arguments:
            stream (BytesIO): stream containing captured image data
        Returns:
            Return False if we detect a shutdown request
        """
        if image is None:
            return None
        # Get the position of the object being tracked
        if self.tracker.Track( image ):
            # Send the coordinates to the robot - if we found our target
            self.robot.TrackObject( *image.track )
        if self.showimages:
            try:
                # Put on the display queue - unless it is full (only takes one).
                otherqueues[0].put_nowait( image )
                return None
            except Queue.Full:
                pass
        return image

    # Setup the camera and run the image capture process
    def run( self ):
        """
        Run the image capture and processing and robot control loop.

        Setups up and executes the multi-threaded image capture and processing.
        """
        # Create a workflow to process images which appear on processingqueue
        self.workflow = workflow.WorkflowManager(
            [
                workflow.WorkerPool(
                    2,                    # Number of worker threads
                    self.imagetracking,   # Function to process the image
                    self.processingqueue, # Input queue for Images
                    self.cameraqueue,     # Output queue for Images
                    (self.displayqueue,)  # Queue for displaying images
                ),
                workflow.WorkerPool(
                    1,                    # Number of worker threads
                    self.displayimage,    # Function to display the image
                    self.displayqueue,    # Input queue for Images to display
                    self.cameraqueue      # Output queue to recycle Images
                ) if self.showimages else None
            ]
        )

        # Start the Camera capture - will run in it's own thread.
        # Captured images will be placed on processingqueue
        # to be processed by self.workflow.
        self.capture = cameracapture.CameraCapture(
            self.camera,
            self.cameraqueue,
            self.processingqueue,
            self.showpreview
        ) if self.camera is not None else None

        # Diagnostic: Print all the threads we have started.
        for t in threading.enumerate():
            print( t )
        print( '' )

    def loop( self ):
        """
        Main execution loop for the robot.
        Reads keypresses and issues commands to control the robot.
        """
        while True:
            time.sleep( 1 )

    def close( self ):
        """
        Close down the robot controller and object tracking components. 
        """
        print( "Closing down robbie..." )
        self.robot.close()
        if self.capture is not None:
            self.capture.close()
        self.workflow.close()
        self.tracker.close()
        s = time.time()
        while threading.active_count() > 2:
            t = time.time()
            if (t - s > 5):
                # If we have bean waiting more than five seconds print out the threads
                # still running.
                for t in threading.enumerate():
                    print( t )
                print( '' )
            time.sleep( 0.5 )

# Local Variables:
# python-indent: 4
# tab-width: 4
# indent-tabs-mode: nil
# End: 
