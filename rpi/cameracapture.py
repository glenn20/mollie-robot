import time
import threading
import Queue

import imageprocessor

class CameraCapture( threading.Thread ):
    """
    Setup the picamera image capture process in a new thread. Images will be captured
    to io.BytesIO streams in Image objects. After capture, Images will be put onto an
    outputqueue (Queue.Queue) ready for further processing....

    Methods:
    run():              Run the continuous image capture process
    close():            Close down the threads and print a report()
    _streamgenerator(): Generator function to yield the next stream for image capture
    """

    def __init__( self,
                  camera,
                  inputqueue,
                  outputqueue,
                  showpreview = False ):
        """
        Constructor for multi-threaded image capture and processing.

        Args:
            camera (picamera.PiCamera): A pre-configured Raspberry PiCamera object
            inputqueue  (Queue.Queue):  Input queue for loading Image objects
            outputqueue (Queue.Queue):  Output queue for Image objects after capture
            showpreview (boolean):      Display image capture preview on RPi console?
        """
        super( CameraCapture, self ).__init__()  # Initialise the super class
        self.camera         = camera
        self.inputqueue     = inputqueue
        self.outputqueue    = outputqueue
        self.showpreview    = showpreview
        self.done           = False            # Flag to shutdown processing
        # Fill the inputqueue Image records - ready for capture
        for i in range(10):
            self.inputqueue.put( imageprocessor.Image() )
        # Call the "run()" method in the new thread
        self.start()

    # Setup the camera and run the image capture process
    def run( self ):
        """
        Run the image capture and processing and robot control loop.

        Setups up and executes the image capture and processing.
        """
        with self.camera as camera:
            if self.showpreview:
                camera.start_preview()
            # time.sleep(2)
            # This is the fastest method to capture a sequence of images from the camera
            # Keep capturing images until self.done is True...
            camera.capture_sequence( self._streamgenerator(), use_video_port=True )

    def close( self ):
        # Stop the image capture
        self.done = True
        # self.join()

    def _streamgenerator( self ):
        """
        Generator to yield the streams for image capture and acquisition.

        Yields the next stream object from the input queue.
        Intended to be used as the stream generator for
        PiCamera.capture_sequence().

        Arguments:
            None
        Yields:
            stream (BytesIO())
        """
        # Get the next Image from the input queue
        # Yield the Image object's stream to the picamera capture-sequence
        # When capture is complete - put the Image on the output queue
        while not self.done:
            try:
                # Get the next Image buffer from the camera queue
                # We use a timeout so we can respond to (self.done=True)
                image = self.inputqueue.get( timeout=0.2 )
            except Queue.Empty:
                continue
            if image is not None:
                image.reset()
                image.time = time.time() # Record the time for the capture
                yield image.stream
                # Resumes when camera asks for next buffer for Image capture
                # Put the just captured Image on the processing queue
                image.stream.seek( 0 )
                self.outputqueue.put( image )

# Local Variables:
# python-indent: 4
# tab-width: 4
# indent-tabs-mode: nil
# End: 
