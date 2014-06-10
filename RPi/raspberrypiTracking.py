# from getch import getch
from rpiarduinocomms import Go, Servo, Track
from colourtracking import SetupColourTracking, ColourTracking
import cv2

speed      = 0
direction  = 0
angle      = 0
trackingOn = False

# Process any key presses - return False if time to quit
def processkeys( c ):
    global speed
    global direction
    global angle
    global trackingOn

    if c < 0:
        return True
    c = chr( c & 255 )  # Bugfix - this is needed for opencv on 64bit linux
    print( ">>", c )
    if c == "q":
        # Quit the program
        Go( 0, 0 )
        # Servo( 0 )
        return False
    elif c == "t":
        # Toggle tracking mode on/off
        trackingOn = not trackingOn
    elif c == " ":
        # Stop robot command
        speed = 0
        direction = 0
        Go( speed, direction )
    elif c == "z":
        # Turn camera/head to left
        angle = angle - 2
        if angle < -90:
            angle = -90
        Servo( angle )
    elif c == "x":
        # Turn camera/head to right
        angle = angle + 2
        if angle > 90:
            angle = 90
        Servo( angle )	
    elif c == "R":
        # Up key - Increase robot speed
        speed = speed+10
        if speed > 255:
            speed = 255
        Go( speed, direction )
    elif c == "T":
        #Down key - Decrease robot speed
        speed = speed-10
        if speed < -255:
            speed = -255
        Go( speed, direction )
    elif c == "S":
        #Right key - Turn robot to the right
        direction = direction+0.02
        if direction > 0.1:
            direction = 0.1
        Go( speed, direction )
    elif c == "Q":
        #Left key - Turn robot to the left
        direction = direction-0.02
        if direction < -0.1:
            direction = -0.1
        Go( speed, direction )
    return True;


def main():
    global trackingOn

    Go( 0, 0 )
    Servo( 0 )
    SetupColourTracking()

    while True:
        track = ColourTracking()
        if track != False:
            posX, posY, area = track
            print( "Track[%s] %d %d %d"
                   % (str(trackingOn), posX, posY, area) )
            if trackingOn == True:
                Track( posX, posY )
        elif trackingOn == True:
            if direction != 0:
                direction = 0
                Go( speed, direction )

        c = cv2.waitKey(50)
        if not processkeys( c ):
            return

    return;

if __name__ == "__main__":
    main()

# Local Variables:
# python-indent: 4
# tab-width: 4
# indent-tabs-mode: nil
# End: 
