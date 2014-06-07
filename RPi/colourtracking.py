import time
import cv2.cv as cv
import cv2
import io
import numpy as np
import picamera

#For Raspberry Pi camera: red ball
#H_MIN =  83
#H_MAX = 188
#S_MIN =  99
#S_MAX = 256
#V_MIN = 109
#V_MAX = 256

#For Raspberry Pi camera: tennis ball
HSV_slice = [
    (  32,  85,  26 ),
    (  50, 256, 256 )
]

capture = 0
width = 320
height = 240
camera = 0

def ColorThreshold(img):
    # returns thresholded image
    imgHSV = cv.CreateImage(cv.GetSize(img), 8, 3)

    # converts BGR image to HSV
    cv.CvtColor(img, imgHSV, cv.CV_BGR2HSV)
    imgProcessed = cv.CreateImage(cv.GetSize(img), 8, 1)

    # converts the pixel values lying within the range to 255 and stores it in the destination
    cv.InRangeS(imgHSV, HSV_slice[0], HSV_slice[1], imgProcessed)
    return imgProcessed

def SetupColourTracking( cvcamera = 0, framewidth = 320, frameheight = 240 ):	
    # captured image size, change to whatever you want
	global capture
	global width
	global height
	global camera
	width = framewidth
	height = frameheight
	#capture = cv.CreateCameraCapture(cvcamera)

    #Over-write default captured image size
	#cv.SetCaptureProperty(capture,cv.CV_CAP_PROP_FRAME_WIDTH,width)
	#cv.SetCaptureProperty(capture,cv.CV_CAP_PROP_FRAME_HEIGHT,height)
	#Create the in-memory stream
	camera = picamera.PiCamera()
	camera.resolution = (width, height)
	camera.start_preview()
	time.sleep(2)

	#cv.NamedWindow( "output", 1 )
	cv.NamedWindow( "processed", 1 )

def ColourTracking():
    global capture
    global width
    global height
    global camera
    stream = io.BytesIO()
    camera.capture(stream, format='jpeg')
	# Construct a numpy array from the stream
    data = np.fromstring(stream.getvalue(), dtype=np.uint8)
	# "Decode" the image from the array, preserving colour
    frame = cv.fromarray( cv2.imdecode(data, 1) )
	# OpenCV returns an array with data in BGR order. 
	
    # Capture the next image from the camera
    #frame = cv.QueryFrame(capture)
    cv.Smooth(frame, frame, cv.CV_BLUR, 3)

    # Generate the thresholded image to identify 
    imgColorProcessed = ColorThreshold(frame)
    mat = cv.GetMat(imgColorProcessed)

    # Calculating the moments
    moments = cv.Moments(mat, 0)
    area = cv.GetCentralMoment(moments, 0, 0)
    moment10 = cv.GetSpatialMoment(moments, 1, 0)
    moment01 = cv.GetSpatialMoment(moments, 0,1)

    # Finding a big enough blob
    if(area > 600000):
        # Calculating the center postition of the blob
        posX = int(moment10 / area)
        posY = int(moment01 / area)
        
        #cv.Circle(frame,(posX,posY),20,(0,255,0),2);
        
        posX = posX - width/2
        posY = posY - height/2
			
	# update video windows
        #cv.ShowImage("processed", imgColorProcessed)
        #cv.ShowImage("output", frame)
        return (posX, posY, area)
    return False

if __name__ == "__main__":
	SetupColourTracking()

	H_MIN = 0
	H_MAX = 256
	S_MIN = 0
	S_MAX = 256
	V_MIN = 0
	V_MAX = 256

	def sethmin( value ):
		global H_MIN 
		H_MIN = value
		HSV_slice[0][0] = value

	def sethmax( value ):
		global H_MAX
		H_MAX = value
		HSV_slice[1][0] = value

	def setsmin( value ):
		global S_MIN
		S_MIN = value
		HSV_slice[0][1] = value

	def setsmax( value ):
		global S_MAX
		S_MAX = value
		HSV_slice[1][1] = value

	def setvmin( value ):
		global V_MIN
		V_MIN = value
		HSV_slice[0][2] = value

	def setvmax( value ):
		global V_MAX
		V_MAX = value
		HSV_slice[1][2] = value

	trackbarWindowName = "Trackbars"
	cv.NamedWindow( trackbarWindowName, 1 )
	cv.CreateTrackbar( "H_MIN", trackbarWindowName, H_MIN, H_MAX, sethmin )
	cv.CreateTrackbar( "H_MAX", trackbarWindowName, H_MAX, H_MAX, sethmax )
	cv.CreateTrackbar( "S_MIN", trackbarWindowName, S_MIN, S_MAX, setsmin )
	cv.CreateTrackbar( "S_MAX", trackbarWindowName, S_MAX, S_MAX, setsmax )
	cv.CreateTrackbar( "V_MIN", trackbarWindowName, V_MIN, V_MAX, setvmin )
	cv.CreateTrackbar( "V_MAX", trackbarWindowName, V_MAX, V_MAX, setvmax )

	while True:
		track = ColourTracking()
		if cv.WaitKey(10) > 0:
			break

