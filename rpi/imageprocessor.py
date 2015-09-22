import io
import time
import threading

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
        self.stream      = io.BytesIO()
        self.event       = threading.Event()
        self.terminated  = False
        self.start()   # Run the "run()" method in a new thread

    def run( self ):
        """
        Execute the image processing function on the stream buffer.
        """
        # This method runs in a separate thread
        while not self.terminated:
            # Wait for an image to be written to the stream
            if self.event.wait(1):
                try:
                    self.stream.seek(0)
                    # Do the image processing
                    self.manager.doImageProcessing( self.stream )
                finally:
                    # Reset the stream and event
                    self.stream.seek(0)
                    self.stream.truncate()
                    self.event.clear()
                    # Return ourselves to the pool
                    self.manager.returntopool( self )

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
    def __init__( self,
                  numberofthreads    = 4,
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
        if (numberofthreads < 2):
            numberofthreads = 2
        elif (numberofthreads > 10):
            numberofthreads = 10
        # Build the pool of Image processing threads
        self.pool = [ImageProcessor( self ) for i in range(numberofthreads)]

    def returntopool( self, processor ):
        """
        Put a thread back into the pool of available processing threads.

        Args:
            processor (ImageProcessor): an image processing thread object 
        """
        with self.lock:
            self.pool.append( processor )

    def doImageProcessing( self, stream ):
        """
        Execute the provided image processing function over the stream.

        Flag shutdown if the image processing function returns False.

        Arguments:
            stream (BytesIO): The stream containing the captured image data
        """
        if self.ProcessingFunc is not None:
            self.done = not self.ProcessingFunc( stream )
        # Print out some progress reports
        self.processedcount += 1
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
            with self.lock:
                processor = (self.pool.pop() if self.pool
                             else None)
            if processor:
                processor.time = time.time() # Record the time for the capture
                yield processor.stream
                # When the camera asks for the next stream wake up the thread
                # owning this Processor
                processor.event.set()
                self.capturecount += 1
                # Print out some progress reports
                # if self.capturecount % 10 == 0:
                #   print( "Capture %03d" % (self.capturecount) )
            else:
                # When the pool is starved, wait a while for it to refill
                time.sleep(0.1)

    def report( self ):
        """
        Print a summary of the image processing statistics.
        """
        print( 'Captured %d images in %d seconds at %.2ffps'
               % (self.capturecount,
                  self.finish-self.start,
                  self.capturecount/(self.finish-self.start) ) )
        print( 'Processed %d images in %d seconds at %.2ffps'
               % (self.processedcount,
                  self.finish-self.start,
                  self.processedcount/(self.finish-self.start) ) )

    # Cleanup all the 
    def close( self ):
        """
        Close down the image processing.
        """
        self.finish = time.time()
        while self.pool:
            with self.lock:
                processor = self.pool.pop()
                processor.terminated = True
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
