import cv2
import numpy as np

HSV_slice = [
    np.array( [   0,   0,   0 ], np.uint8 ),
    np.array( [ 179, 255, 255 ], np.uint8 )
]

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

def SetupHSVTuning( hsv_slice = None ):
    global SHOW_IMAGES
    global HSV_slice

    SHOW_IMAGES = True
    if hsv_slice is not None:
        HSV_slice = hsv_slice

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

SHOW_IMAGES = False

# Setup for colour tracking
def SetupColourTracking( hsv_slice    = None,
                         show_images  = False ):
    global SHOW_IMAGES
    global HSV_slice

    SHOW_IMAGES = show_images
    if hsv_slice is not None:
        HSV_slice = hsv_slice
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
        None, iterations=8 )

# Read an image from the stream
# Return the coordinates and area of the object (posx, posy, area)
# The centre of the image is at (posx, posy) = (0, 0)
def ColourTracking( stream, use_contours = True ):
    global SHOW_IMAGES
    
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
    if not use_contours:
        # Calculate moments from threshold image
        moments = cv2.moments( imgFiltered, 0 )
    else:
        # Calculate moments from the largest contour of the threshold image
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
    if(area > 0):
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

    return (posX, posY, area)
