import io
import time
import threading
import cv2
import numpy as np

import picamera

# from getch import getch
import arduinocomms
import arduinorobot
import colourtracker
import imageprocessor

width           = 320
height          = 240

# Create a Robot instance talking on the arduino Bus
# I2Cbusnumber = 0 if RPI version 1 else 1 if RPI version 2
# I2Caddress of the arduino board - as set in arduinoRobot program
arduinocomms = arduinocomms.ArduinoComms( 
    I2Cbusnumber = 1,    
    I2Caddress   = 0x04,
    dummy        = True
    )
robot = arduinorobot.ArduinoRobot( arduinocomms )

hsv_all = [
    np.array( [   0,   0,   0 ], np.uint8 ),
    np.array( [ 179, 255, 255 ], np.uint8 )
]

hsv_tennisball = [
    np.array( [  32,  85,  26 ], np.uint8 ),
    np.array( [  50, 255, 255 ], np.uint8 )
]

hsv_redball = [
    np.array( [   0, 223,  60 ], np.uint8 ),
    np.array( [   9, 255, 255 ], np.uint8 )
]

objecttracker = colourtracker.ColourTracker(
    hsv_slice           = hsv_redball,
    use_contours        = True,
    show_images         = True,
    tune_hsv_thresholds = False
    )

# Initialise globals

def doColourTracking( stream ):
    global robot, objecttracker

    ok = True
    # Get the postion of the object being tracked
    posX, posY, area = objecttracker.Track( stream )
    robot.TrackObject( posX, posY, area )
    # Process any key presses until none left
    c = 0
    i = 0
    while ok and c >= 0 and i < 5:
        i += 1
        c = cv2.waitKey( 50 )
        ok = robot.RemoteControl( c ) is not None
    return ok

def rpitracking():
    global width, height

    with picamera.PiCamera() as camera:
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

        processors = imageprocessor.ProcessorManager(
            numberofthreads         = 4,
            imageprocessingfunction = doColourTracking
            )
        camera.capture_sequence( processors.streams(), use_video_port=True )
        processors.close()

def main():
    global objecttracker

    try:
        rpitracking()
    except KeyboardInterrupt:
        print( "Interrupted - shutting down..." )
    
    try:
        cv2.destroyAllWindows()
    except KeyboardInterrupt:
        print( "Exiting..." )

    if objecttracker.tune_hsv_thresholds:
        print( "HSV_min = %03d %03d %03d"
               % (objecttracker.HSV_slice[0][0],
                  objecttracker.HSV_slice[0][1],
                  objecttracker.HSV_slice[0][2]) )
        print( "HSV_max = %03d %03d %03d"
               % (objecttracker.HSV_slice[1][0],
                  objecttracker.HSV_slice[1][1],
                  objecttracker.HSV_slice[1][2]) )

if __name__ == "__main__":
    main()

# Local Variables:
# python-indent: 4
# tab-width: 4
# indent-tabs-mode: nil
# End: 
