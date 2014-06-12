import io
import time
import threading

# Each ImageProcessor has a stream for capturing images from the camera
# and has a thread for processing it in the run() method.
class ImageProcessor( threading.Thread ):
    def __init__( self, processormanager ):
        super( ImageProcessor, self ).__init__()
        self.processormanager = processormanager
        self.stream = io.BytesIO()
        self.event = threading.Event()
        self.terminated = False
        self.start()   # Run the "run()" method in a new thread

    def run(self):
        # This method runs in a separate thread
        while not self.terminated:
            # Wait for an image to be written to the stream
            if self.event.wait(1):
                try:
                    self.stream.seek(0)
                    # Do the image processing
                    self.processormanager.doImageProcessing( self.stream )
                finally:
                    # Reset the stream and event
                    self.stream.seek(0)
                    self.stream.truncate()
                    self.event.clear()
                    # Return ourselves to the pool
                    self.processormanager.returntopool( self )

# We will build a pool of Imageprocessors
class ProcessorManager():
    def __init__( self,
                  numberofthreads    = 4,
                  processingfunction = None ):
        self.ProcessingFunc = processingfunction
        self.done           = False
        self.lock           = threading.Lock()
        self.capturecount   = 0
        self.processedcount = 0
        self.start          = time.time()
        self.finish         = time.time()
        if (numberofthreads < 2):
            numberofthreads = 2
        elif (numberofthreads > 10):
            numberofthreads = 10
        else:
            numberofthreads = 4
        # Build the pool of Image processing threads
        self.pool = [ImageProcessor( self ) for i in range(numberofthreads)]

    def returntopool( self, processor ):
        with self.lock:
            self.pool.append( processor )

    def doImageProcessing( self, stream ):
        if self.ProcessingFunc is not None:
            if not self.ProcessingFunc( stream ):
                self.done = True
        # Print out some progress reports
        self.processedcount += 1
        if (self.processedcount % 10 == 0):
            print( "Processed %4d" % (self.processedcount) )

    # A generator function to yield the streams from the pool of ImageProcessors
    # for successive image capture by camera.capture_sequence() (see below) 
    #  - Get the next ImageProcessor from the pool
    #  - Yield the ImageProcessor's stream to the picamera capture-sequence
    #  - Then wake up the ImageProcessor's thread to start processing
    def streams( self ):
        while not self.done:
            with self.lock:
                if self.pool:
                    processor = self.pool.pop()
                else:
                    processor = None
            if processor:
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
        # Print a summary of what we have done
        print( 'Captured %d images in %d seconds at %.2ffps'
               % (self.capturecount,
                  self.finish-self.start,
                  self.capturecount/(self.finish-self.start) ) )
        print( 'Processed %d images in %d seconds at %.2ffps'
               % (self.processedcount,
                  self.finish-self.start,
                  self.processedcount/(self.finish-self.start) ) )

    def close( self ):
        self.finish = time.time()
        while self.pool:
            with self.lock:
                processor = self.pool.pop()
                processor.terminated = True
                processor.join()
        self.report()

    def __enter__( self ):
        return self

    def __exit__( self, exc_type, exc_val, exc_tb ):
        self.close()

# Local Variables:
# python-indent: 4
# tab-width: 4
# indent-tabs-mode: nil
# End: 
