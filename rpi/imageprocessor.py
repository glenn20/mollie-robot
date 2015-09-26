import io
import time
import threading
import Queue

# Each ImageProcessor has a stream for capturing images from the camera
# and has a thread for processing it in the run() method.
class Image():
    """
    Multi-threaded processing of images.

    This is a helper class for the ProcessorManager class

    Methods:
    __init__( manager ): Make a processing thread with link back to the manager
    run() : Run an image processing function over images as they are acquired
    """
    def __init__( self ):
        """
        Construct an Image processing thread with a link back to the Manager

        Args:
            processormanager (ProcessorManager)
        """
        self.stream   = io.BytesIO()
        self.img      = None
        self.contours = None
        self.bestcountour = None
        self.time     = None
        self.location = (None, None)
        self.track    = (None, None, None)


# Each ImageProcessor has a stream for capturing images from the camera
# and has a thread for processing it in the run() method.
class ImageProcessor( threading.Thread ):
    """
    Multi-threaded processing of images.

    This is a helper class for the ProcessorManager class

    Methods:
    __init__( manager ): Make a processing thread with link back to the manager
    run() : Run an image processing function over images as they are acquired
    """
    def __init__( self, manager ):
        """
        Construct an Image processing thread with a link back to the Manager

        Args:
            processormanager (ProcessorManager)
        """
        super( ImageProcessor, self ).__init__()
        self.manager     = manager
        self.start()   # Run the "run()" method in a new thread

    def run( self ):
        """
        Execute the image processing function on the stream buffer.
        """
        # This method runs in a separate thread
        while True:
            # Wait for an image to appear on the processing queue
            image = self.manager.processingqueue.get()
            # If this is a sentinel - put it back on the queue and end thread.
            if (image is self.manager.sentinel):
                self.manager.processingqueue.put( self.manager.sentinel )
                break
            try:
                image.stream.seek(0)
                # Do the image processing
                self.manager.doImageProcessing( image )
            finally:
                # Reset the stream and event
                image.stream.seek(0)
                image.stream.truncate()
                # Put the image back on the camera queue
                self.manager.cameraqueue.put( image )

# We will build a pool of Imageprocessors
class ProcessorManager():
    """
    Setup and run a pool of threads to process images as they are acquired.

    Can be invoked as a ContextManager object.

    Methods:
    streamgenerator(): Generator function to yield the next stream for image capture
    report():  Print a summary of the image capture and processing stats
    close():   Close down the threads and print a report()

    Methods called back from the ImageProcessor threads:
    doImageProcessing(stream): Execute image processing function on the stream
    returntopool(processor)  : Return this thread to the active queue
    """

    # used to signal threads to stop processing
    sentinel = object()

    def __init__( self,
                  numberofthreads    = 2,
                  processingfunction = None ):
        """
        Construct a Manager for multi-threaded image capture and processing.

        Can be invoked as a ContextManager.

        Args:
            numberofthreads (int, default=4): Number of threads to use
            processingfunction: Callback function to execute image processing.
                                Should return False to signal shutdown.
        """
        self.ProcessingFunc = processingfunction
        self.done           = False            # Flag to shutdown processing
        self.lock           = threading.Lock() # Lock for the thread pool
        self.capturecount   = 0                # Number of captured images
        self.processedcount = 0                # Number of processed images
        self.start          = time.time()      # Time we started capture
        self.finish         = time.time()      # Time we finished processing
        # Build the pool of Image objects
        if (numberofthreads < 1):
            numberofthreads = 1
        elif (numberofthreads > 10):
            numberofthreads = 10
        # Create the queues for the "Image"s first
        self.cameraqueue = Queue.Queue()
        self.processingqueue = Queue.Queue()
        # Then build the pool of Image images
        self.imagepool = [Image() for i in range( 2 * numberofthreads )]
        # Fill the camera queue with Image records - ready for capture
        for i in self.imagepool:
            self.cameraqueue.put( i )
        # Create the pool of image processing threads
        self.processingpool = [ImageProcessor( self )
                               for i in range(numberofthreads)]


    def doImageProcessing( self, image ):
        """
        Execute the provided image processing function over the stream.

        Flag shutdown if the image processing function returns False.

        Arguments:
            stream (BytesIO): The stream containing the captured image data
        """
        if self.ProcessingFunc is not None:
            self.done = not self.ProcessingFunc( image )
        self.processedcount += 1
        # Print out some progress reports
        # if (self.processedcount % 10 == 0):
        #     print( "Processed %4d" % (self.processedcount) )

    def streamgenerator( self ):
        """
        Generator to yield the streams for image capture and acquisition.

        Yields the stream object from the next available processing thread.
        Intended to be used as the stream generator for
        PiCamera.capture_sequence().

        Arguments:
            None
        Yields:
            stream (BytesIO())
        """
        # Get the next ImageProcessor from the pool
        # Yield the ImageProcessor's stream to the picamera capture-sequence
        # Then wake up the ImageProcessor's thread to start processing
        while not self.done:
            try:
                # Get the next Image buffer from the camera queue
                image = self.cameraqueue.get( timeout=0.1 )
            except Queue.Empty:
                continue
            if image:
                image.time = time.time() # Record the time for the capture
                yield image.stream
                # Resumes when camera asks for next buffer for Image capture
                # Put the just captured Image on the processing queue
                self.processingqueue.put( image )
                self.capturecount += 1
                # Print out some progress reports
                # if self.capturecount % 10 == 0:
                #   print( "Capture %03d" % (self.capturecount) )
        # Add the sentinel to the processing queue..
        # ...tells the processing threads to shutdown
        self.processingqueue.put( self.sentinel )

    def report( self ):
        """
        Print a summary of the image processing statistics.
        """
        print( "\r\n" )
        print( 'Captured %d images in %d seconds at %.2ffps\r\n'
               % (self.capturecount,
                  self.finish-self.start,
                  self.capturecount/(self.finish-self.start) ) )
        print( 'Processed %d images in %d seconds at %.2ffps\r\n'
               % (self.processedcount,
                  self.finish-self.start,
                  self.processedcount/(self.finish-self.start) ) )

    # Cleanup up all the 
    def close( self ):
        """
        Close down the image processing.
        """
        self.finish = time.time()
        for processor in self.processingpool:
            processor.join()
        self.report()

    # So we can use this class as a ContextManager
    def __enter__( self ):
        return self

    def __exit__( self, exc_type, exc_val, exc_tb ):
        self.close()

# Local Variables:
# python-indent: 4
# tab-width: 4
# indent-tabs-mode: nil
# End: 
