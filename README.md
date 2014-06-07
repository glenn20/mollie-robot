mollie-robot
============

Mollie's robot code

Software for Mollie's raspberry pi robot

arduinoRobot/

  Arduino code to run on the Arduino Uno microcontroller.

    - Receive commands from raspberry pi over IC serial bus
    - Use PID control to track the left and right wheel movements
    - Use PID control for the robot head (camera) servos to track the
      desired object
    - PID control of the direction of travel so the robot follows the
      tracked object

RPi/

  Python code to run on the Raspberry Pi computer:

    - Capture images from pi-camera and perform the colour threshold
      image processing to track object (eg. a tennis ball).
    - Keyboard control of the robot
    - Send object tracking instructions to the arduino to control the robot.
