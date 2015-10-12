import numpy as np
import cv2

class ColourTracker():
    HSV_all = [np.array( [   0,   0,   0 ], np.uint8 ),
               np.array( [ 179, 255, 255 ], np.uint8 )]

    def __init__( self,
                  hsv_slice    = None,
                  use_contours = True,
                  show_images  = False,
                  tune_hsv     = False
                  ):
        self.HSV_slice    = (hsv_slice
                             if hsv_slice is not None
                             else self.__class__.HSV_all)
        self.use_contours = use_contours
        self.show_images  = show_images or tune_hsv
        self.tune_hsv     = tune_hsv
        if self.show_images:
            cv2.namedWindow( "output", 1 )
        if self.tune_hsv:
            self.SetupHSVTuning()

    # return a monochrome image with only pixels between the HSV range
    def _ColorThreshold( self, img ):
        return cv2.dilate(
            cv2.erode(
                cv2.inRange(
                    cv2.cvtColor( img, cv2.COLOR_BGR2HSV ),
                    self.HSV_slice[0], self.HSV_slice[1] ),
                None, iterations=3 ),
            None, iterations=8 )

    # Calculate moments from the largest contour of the thresholded image
    def _getMoments( self, imgFiltered, image ):
        # If use_contours is "off": calculate moments from threshold image
        if not self.use_contours:
            return cv2.moments( imgFiltered, 0 )

        image.contours, _ = cv2.findContours( imgFiltered,
                                              cv2.RETR_EXTERNAL,
                                              cv2.CHAIN_APPROX_SIMPLE )
        if not image.contours:
            return False
        # Get the contour with maximum area
        image.bestcontour = max( image.contours, key=lambda c: cv2.contourArea( c ) )
        return cv2.moments( image.bestcontour, 0 )

    # Read an image from the stream
    # Return the coordinates and area of the object (posx, posy, area)
    # The centre of the image is at (posx, posy) = (0, 0)
    def Track( self, image ):
        # "Decode" the image from the array, preserving colour
        image.img = cv2.imdecode( np.fromstring( image.stream.getvalue(),
                                                 dtype=np.uint8 ), 1 )
        
        # Camera applies gaussian blur - no need to do it again.
        # image.img = cv2.smooth( image.img, cv2.BLUR, 3 )

        # Generate the thresholded image to identify 
        imgFiltered = self._ColorThreshold( image.img )

        # Calculate moments from the largest contour, or the threshold image
        moments = self._getMoments( imgFiltered, image )
        if not moments:
            return False

        area = moments['m00']
        if (area < 1):
            return False

        image.location = (None,None)
        image.track    = (None,None,None)
        # Calculate the centre of gravity of the largest "blob" of colour we found
        image.location = (int(moments['m10'] / area),
                          int(moments['m01'] / area))
        # Convert coords so (0,0) is at centre of image and Y is upward
        image.track = (+(image.location[0] - image.img.shape[1]/2),
                       -(image.location[1] - image.img.shape[0]/2),
                       area)

        # Return the coords of the object and it's area
        return True

    def Showimage( self, image ):
        if image.contours:
            # Draw all the contours in red
            cv2.drawContours( image.img, image.contours,     -1, (0,0,255), 2 )
            # Draw the max contour in blue
            cv2.drawContours( image.img, [image.bestcontour], 0, (255,0,0), 2 )

        if image.location[0] is not None:
            # Draw a circle around the tracking point
            cv2.circle( image.img, image.location, 20, (0,255,0), 2 );

        # update display windows
        cv2.imshow( "output", image.img )
        # Need this to make the display update (also adds pause to thread)
        cv2.waitKey( 100 )
        return

    def SetHSVSlice( self, hsv_slice ):
        if hsv_slice is not None:
            self.HSV_slice = hsv_slice

    def setHmin( self, value ):
        self.HSV_slice[0][0] = value
    
    def setHmax( self, value ):
        self.HSV_slice[1][0] = value
        
    def setSmin( self, value ):
        self.HSV_slice[0][1] = value

    def setSmax( self, value ):
        self.HSV_slice[1][1] = value

    def setVmin( self, value ):
        self.HSV_slice[0][2] = value

    def setVmax( self, value ):
        self.HSV_slice[1][2] = value

    def SetupHSVTuning( self, hsv_slice = None ):
        self.show_images = True
        if hsv_slice is not None:
            self.HSV_slice = hsv_slice

        windowname = "Trackbars"
        cv2.namedWindow( windowname, 1 )
        cv2.createTrackbar( "H_MIN", windowname, 0, 180, self.setHmin )
        cv2.createTrackbar( "H_MAX", windowname, 0, 180, self.setHmax )
        cv2.createTrackbar( "S_MIN", windowname, 0, 255, self.setSmin )
        cv2.createTrackbar( "S_MAX", windowname, 0, 255, self.setSmax )
        cv2.createTrackbar( "V_MIN", windowname, 0, 255, self.setVmin )
        cv2.createTrackbar( "V_MAX", windowname, 0, 255, self.setVmax )

        cv2.setTrackbarPos( "H_MIN", windowname, self.HSV_slice[0][0] )
        cv2.setTrackbarPos( "H_MAX", windowname, self.HSV_slice[1][0] )
        cv2.setTrackbarPos( "S_MIN", windowname, self.HSV_slice[0][1] )
        cv2.setTrackbarPos( "S_MAX", windowname, self.HSV_slice[1][1] )
        cv2.setTrackbarPos( "V_MIN", windowname, self.HSV_slice[0][2] )
        cv2.setTrackbarPos( "V_MAX", windowname, self.HSV_slice[1][2] )

    def close( self ):
        # If we were adjusting HSV slice values, print them out
        if self.tune_hsv:
            print( "HSV_min = %03d %03d %03d"
                   % (self.HSV_slice[0][0],
                      self.HSV_slice[0][1],
                      self.HSV_slice[0][2]) )
            print( "HSV_max = %03d %03d %03d"
                   % (self.HSV_slice[1][0],
                      self.HSV_slice[1][1],
                      self.HSV_slice[1][2]) )

        # Wind up and shutdown
        if self.show_images:
            print( "Closing down windows..." )
            cv2.destroyAllWindows()
