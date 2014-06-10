import io
import time
import threading
import cv2
import numpy as np

import picamera

# from getch import getch
from robot import Robot
from colourtracking import SetupColourTracking, ColourTracking, SetupHSVTuning

TUNE_THRESHOLDS = False
PROCESS_ROBOT   = False

width          = 320
height         = 240

hsv_all = [
    np.array( [   0,   0,   0 ], np.uint8 ),
    np.array( [ 179, 255, 255 ], np.uint8 )
]

#For Raspberry Pi camera: tennis ball
hsv_tennisball = [
    np.array( [  32,  85,  26 ], np.uint8 ),
    np.array( [  50, 255, 255 ], np.uint8 )
]

hsv_redball = [
    np.array( [   0, 223,  60 ], np.uint8 ),
    np.array( [   9, 255, 255 ], np.uint8 )
]

hsv_slice = hsv_redball

# Initialise globals

# Create a pool of image processors
done = False
lock = threading.Lock()
pool = []

capturecount   = 0
processedcount = 0
start          = time.time()
finish         = time.time()

robot = Robot()

def doColourTracking( stream ):
    global robot

    ok = True
    # Get the postion of the object being tracked
    posX, posY, area = ColourTracking( stream )
    robot.TrackObject( posX, posY, area )
    # Process any key presses until none left
    c = 0
    i = 0
    while ok and c >= 0 and i < 5:
        i += 1
        c = cv2.waitKey( 50 )
        ok = robot.RemoteControl( c )
    return ok

# We will build a pool of Imageprocessors
# Each ImageProcessor has a stream for capturing images from the camera
# and has a thread for processing it in the run() method.
class ImageProcessor(threading.Thread):
    def __init__(self):
        super(ImageProcessor, self).__init__()
        self.stream = io.BytesIO()
        self.event = threading.Event()
        self.terminated = False
        self.start()   # Run the "run()" method in a new thread

    def run(self):
        # This method runs in a separate thread
        global done, pool, lock, finish, processedcount
        while not self.terminated:
            # Wait for an image to be written to the stream
            if self.event.wait(1):
                try:
                    self.stream.seek(0)
                    # Do the image processing
                    if not doColourTracking( self.stream ):
                        done = True
                    # Print out some progress reports
                    processedcount += 1
                    if (processedcount % 10 == 0):
                        print( "Processed %4d" % (processedcount) )
                    finish = time.time()
                finally:
                    # Reset the stream and event
                    self.stream.seek(0)
                    self.stream.truncate()
                    self.event.clear()
                    # Return ourselves to the pool
                    with lock:
                        pool.append(self)

# A generator function to yield the streams from the pool of ImageProcessors
# for successive image capture by camera.capture_sequence() (see below) 
#  - Get the next ImageProcessor from the pool
#  - Yield the ImageProcessor's stream to the picamera capture-sequence
#  - Then wake up the ImageProcessor's thread to start processing
def streams():
    global pool, lock, done, capturecount
    while not done:
        with lock:
            if pool:
                processor = pool.pop()
            else:
                processor = None
        if processor:
            yield processor.stream
            # When the camera asks for the next stream wake up the thread
            # owning this Processor
            processor.event.set()
            capturecount += 1
            # if capturecount % 10 == 0:
            #   print( "Capture %03d" % (capturecount) )
        else:
            # When the pool is starved, wait a while for it to refill
            time.sleep(0.1)

def poolcleanup():
    global pool, lock

    while pool:
        with lock:
            processor = pool.pop()
            processor.terminated = True
            processor.join()

def rpitracking():
    global width, height, pool, hsv_slice, start

    SetupColourTracking( hsv_slice, show_images = True )
    if TUNE_THRESHOLDS:
        SetupHSVTuning( hsv_slice )
    
    with picamera.PiCamera() as camera:
        pool = [ImageProcessor() for i in range(4)]
        camera.preview_fullscreen = False
        camera.preview_window = (100, 100, width, height )
        camera.resolution = (width, height)
        camera.framerate = 10
        #camera.exposure_mode = 'off'
        camera.ISO = 800
        camera.image_effect = 'blur'
        camera.awb_mode = 'off' # 'fluorescent'
        camera.awb_gains = (1.2,1.2)
        camera.start_preview()
        #time.sleep(2)
        start = time.time()
        camera.capture_sequence( streams(), use_video_port=True )

def main():
    global start, finish, capturecount, processedcount

    try:
        rpitracking()
    except KeyboardInterrupt:
        print( "Interrupted - shutting down..." )
    
    # Print a summary of what we have done
    print( 'Captured %d images in %d seconds at %.2ffps'
           % (capturecount, finish-start, capturecount/(finish-start) ) )
    print( 'Processed %d images in %d seconds at %.2ffps'
           % (processedcount, finish-start, processedcount/(finish-start) ) )
        
    if TUNE_THRESHOLDS:
        print( "HSV_min = %03d %03d %03d"
               % (HSV_slice[0][0], HSV_slice[0][1], HSV_slice[0][2]) )
        print( "HSV_max = %03d %03d %03d"
               % (HSV_slice[1][0], HSV_slice[1][1], HSV_slice[1][2]) )

    try:
        cv2.destroyAllWindows()
        poolcleanup()
    except KeyboardInterrupt:
        print( "Exiting..." )


if __name__ == "__main__":
    main()

# Local Variables:
# python-indent: 4
# tab-width: 4
# indent-tabs-mode: nil
# End: 
