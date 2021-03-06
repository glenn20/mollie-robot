#!/usr/bin/env python

import argparse
import sys

parser = argparse.ArgumentParser(
    description='Run the RPI-arduino robot control program.'
    )
parser.add_argument(
    "--tunehsv", action="store_true",
    help="Let us adjust the HSV values for the colour tracking"
    )
parser.add_argument(
    "--show", action="store_true",
    help="Display the captured and processed images for tracking"
    )
parser.add_argument(
    "--preview", action="store_true",
    help="Show the pi-camera preview window"
    )
parser.add_argument(
    "--nocamera", action="store_true",
    help="Do not use the Raspberry Pi camera"
    )
parser.add_argument(
    "--nocontours", action="store_true",
    help="Eliminate the contour finding part of the object tracking"
    )
parser.add_argument(
    "--threads", type=int, default=2, choices=[1,2,3,4,5,6,7,8],
    help="Number of threads to use for the image processing"
    )
args = parser.parse_args()

import picamera

import arduinorobot
import colourtracker
import trackingrobot
import hsvvalues

# Create a Robot instance
robot = arduinorobot.ArduinoRobot()

# Create a ColourTracker instance to track objects
tracker = colourtracker.ColourTracker(
    hsv_slice        = hsvvalues.hsvvalues["bluething"],
    use_contours     = not args.nocontours,
    show_images      = args.show,
    tune_hsv         = args.tunehsv
    )

# Setup the Raspberry Pi camera
camera = None
if not args.nocamera:
    camera = picamera.PiCamera()
    resolution = (320, 240)
    camera.preview_fullscreen = False
    camera.preview_window     = (100, 100, resolution[0], resolution[1])
    camera.resolution         = resolution
    camera.framerate          = 10
    # camera.exposure_mode    = 'off'
    camera.iso                = 800
    camera.image_effect       = 'blur'
    camera.awb_mode           = 'off' # 'fluorescent'
    camera.awb_gains          = (1.2,1.2)

# Connect to and initialise the arduino robot
robot.initialise()

# Assemble our combined TrackingRobot from a robot, a camera and a tracker 
trackingrobot = trackingrobot.TrackingRobot(
    robot            = robot,
    tracker          = tracker,
    camera           = camera,
    show_images      = args.show,
    showpreview      = args.preview
    )

# Now turn on the robot which will:
# - Capture images from the camera
# - Use color slicing to locate objects in the images (multithreaded)
# - Send the coords of the object to the arduino controlling the robot
# - trap any keypresses and use them as remotecontrol for the robot
try:
    trackingrobot.run()
    trackingrobot.loop()
    trackingrobot.close()
except (KeyboardInterrupt, SystemExit):
    trackingrobot.close()
    sys.exit()

# Local Variables:
# python-indent: 4
# tab-width: 4
# indent-tabs-mode: nil
# End: 
