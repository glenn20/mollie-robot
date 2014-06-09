import io
import time
import threading
import cv2
import numpy as np

import picamera

TUNE_THRESHOLDS = False
USE_CONTOURS    = True
SHOW_IMAGES     = False

if TUNE_THRESHOLDS:
    SHOW_IMAGES = True

# Create a pool of image processors
done = False
lock = threading.Lock()
pool = []

capturecount   = 0
processedcount = 0
start          = time.time()
finish         = time.time()
runtime        = 30

width          = 320
height         = 240
camera         = 0

#For Raspberry Pi camera: tennis ball
HSV_tennisball = [
    np.array( [  32,  85,  26 ], np.uint8 ),
    np.array( [  50, 256, 255 ], np.uint8 )
]

HSV_redball = [
    np.array( [   0, 223,  60 ], np.uint8 ),
    np.array( [   9, 255, 255 ], np.uint8 )
]

HSV_slice = HSV_redball

def SetupColourTracking():
    global SHOW_IMAGES
    # captured image size, change to whatever you want
    if SHOW_IMAGES:
        cv2.namedWindow( "output", 1 )
        cv2.namedWindow( "processed", 1 )

# return a monchrome image with only pixels between the HSV range
def ColorThreshold( img ):
    global HSV_slice
    return cv2.dilate(
        cv2.erode(
            cv2.inRange(
                cv2.cvtColor( img, cv2.COLOR_BGR2HSV ),
                HSV_slice[0], HSV_slice[1] ),
            None, iterations=3 ),
        None, iterations=10 )

def ColourTracking( stream ):
    global SHOW_IMAGES
    global USE_CONTOURS
    
    # "Decode" the image from the array, preserving colour
    data = np.fromstring( stream.getvalue(), dtype=np.uint8 )
    img = cv2.imdecode( data, 1 )

    # Camera applies gaussian blur - no need to do it again.
    # img = cv2.smooth( img, cv2.BLUR, 3 )

    # Generate the thresholded image to identify 
    imgFiltered = ColorThreshold( img )

    if SHOW_IMAGES:
        cv2.imshow( "processed", imgFiltered )

    posX = 0
    posY = 0

    moments = None
    if not USE_CONTOURS:
        # Calculate moments from threshold image
        moments = cv2.moments( imgFiltered, 0 )
    else:
        # Calculate moments from contours of the threshold image
        contours, heirarchy = cv2.findContours( imgFiltered,
                                                cv2.RETR_EXTERNAL,
                                                cv2.CHAIN_APPROX_SIMPLE )
        if SHOW_IMAGES:
            cv2.drawContours( img, contours, -1, (0,0,255), 2 ) 
        maxarea = 0
        bestcontour = None
        ball = None
        for contour in contours:
            area = cv2.contourArea( contour )
            if (area > maxarea):
                maxarea = area
                bestcontour = contour
        if bestcontour is not None:
            moments = cv2.moments( bestcontour, 0 )
            if SHOW_IMAGES:
                cv2.drawContours( img, [bestcontour], 0, (255,0,0), 2 ) 

    area = 0
    if moments is not None:
        area = moments['m00']

    # Finding a big enough blob
    if(area > 50):
        # Calculating the center postition of the object
        posX = int(moments['m10'] / area)
        posY = int(moments['m01'] / area)

        if SHOW_IMAGES:
            cv2.circle( img, (posX, posY), 20, (0,255,0), 2 );

        posX =   posX - img.shape[1]/2
        posY = -(posY - img.shape[0]/2)

    # update video windows
    if SHOW_IMAGES:
        cv2.imshow( "output", img )
        if cv2.waitKey( 50 ) > 0:
            return False

    return (posX, posY, area)

class ImageProcessor(threading.Thread):
    def __init__(self):
        super(ImageProcessor, self).__init__()
        self.stream = io.BytesIO()
        self.event = threading.Event()
        self.terminated = False
        self.start()

    def run(self):
        # This method runs in a separate thread
        global done
        global start, finish, processedcount
        while not self.terminated:
            # Wait for an image to be written to the stream
            if self.event.wait(1):
                try:
                    self.stream.seek(0)
                    coords = ColourTracking( self.stream )
                    done = (coords == False)
                    processedcount += 1
                    if (processedcount % 10 == 0 and not done):
                        print( "Processed %4d, Pos = %3d %3d %d"
                               % (processedcount, coords[0], coords[1], coords[2] ) )
                    finish = time.time()
                finally:
                    # Reset the stream and event
                    self.stream.seek(0)
                    self.stream.truncate()
                    self.event.clear()
                    # Return ourselves to the pool
                    with lock:
                        pool.append(self)

def streams():
    global done, capturecount
    while not done:
        with lock:
            if pool:
                processor = pool.pop()
            else:
                processor = None
        if processor:
            yield processor.stream
            processor.event.set()
        else:
            # When the pool is starved, wait a while for it to refill
            time.sleep(0.1)
        capturecount += 1
#        if capturecount % 10 == 0:
#            print( "Capture %03d" % (capturecount) )

if __name__ == "__main__":
    if TUNE_THRESHOLDS:
        def sethmin( value ):
            global HSV_slice 
            HSV_slice[0][0] = value
    
        def sethmax( value ):
            global HSV_slice 
            HSV_slice[1][0] = value
        
        def setsmin( value ):
            global HSV_slice 
            HSV_slice[0][1] = value

        def setsmax( value ):
            global HSV_slice 
            HSV_slice[1][1] = value

        def setvmin( value ):
            global HSV_slice 
            HSV_slice[0][2] = value

        def setvmax( value ):
            global HSV_slice 
            HSV_slice[1][2] = value

        trackbarWindowName = "Trackbars"
        cv2.namedWindow( trackbarWindowName, 1 )
        cv2.createTrackbar( "H_MIN", trackbarWindowName, 0, 180, sethmin )
        cv2.setTrackbarPos( "H_MIN", trackbarWindowName, HSV_slice[0][0] )
        cv2.createTrackbar( "H_MAX", trackbarWindowName, 0, 180, sethmax )
        cv2.setTrackbarPos( "H_MAX", trackbarWindowName, HSV_slice[1][0] )
        cv2.createTrackbar( "S_MIN", trackbarWindowName, 0, 255, setsmin )
        cv2.setTrackbarPos( "S_MIN", trackbarWindowName, HSV_slice[0][1] )
        cv2.createTrackbar( "S_MAX", trackbarWindowName, 0, 255, setsmax )
        cv2.setTrackbarPos( "S_MAX", trackbarWindowName, HSV_slice[1][1] )
        cv2.createTrackbar( "V_MIN", trackbarWindowName, 0, 255, setvmin )
        cv2.setTrackbarPos( "V_MIN", trackbarWindowName, HSV_slice[0][2] )
        cv2.createTrackbar( "V_MAX", trackbarWindowName, 0, 255, setvmax )
        cv2.setTrackbarPos( "V_MAX", trackbarWindowName, HSV_slice[1][2] )

    with picamera.PiCamera() as camera:
        pool = [ImageProcessor() for i in range(8)]
        camera.preview_fullscreen = False
        camera.preview_window = (1500, 100, width, height )
        camera.resolution = (width, height)
        camera.framerate = 10
        #camera.exposure_mode = 'off'
        camera.ISO = 800
        camera.image_effect = 'blur'
        camera.awb_mode = 'off' # 'fluorescent'
        camera.awb_gains = (1.2,1.2)
        camera.start_preview()
        #time.sleep(2)
        SetupColourTracking()
        start = time.time()
        camera.capture_sequence(streams(), use_video_port=True)

    # Shut down the processors in an orderly fashion
    while pool:
        with lock:
            processor = pool.pop()
            processor.terminated = True
            processor.join()

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

    #cv2.destroyAllWindows()

