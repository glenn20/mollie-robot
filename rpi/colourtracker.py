import cv2
import numpy as np

class ColourTracker():
    def __init__( self,
                  hsv_slice           = None,
                  use_contours        = True,
                  show_images         = False,
                  tune_hsv_thresholds = False
                  ):
        self.HSV_all = [np.array( [   0,   0,   0 ], np.uint8 ),
                        np.array( [ 179, 255, 255 ], np.uint8 )]
        self.HSV_slice = self.HSV_all
        if hsv_slice is not None:
            self.HSV_slice = hsv_slice
        self.use_contours = use_contours
        self.show_images = show_images
        self.tune_hsv_thresholds = tune_hsv_thresholds
        cv2.namedWindow( "output", 1 )
        if self.show_images:
            cv2.namedWindow( "processed", 1 )
        if self.tune_hsv_thresholds:
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
    def _getContourMoments( self, imgFiltered, img ):
        contours, heirarchy = cv2.findContours( imgFiltered,
                                                cv2.RETR_EXTERNAL,
                                                cv2.CHAIN_APPROX_SIMPLE )
        if self.show_images:
            cv2.drawContours( img, contours, -1, (0,0,255), 2 ) 
        maxarea = 0
        bestcontour = None
        moments = None
        for contour in contours:
            area = cv2.contourArea( contour )
            if (area > maxarea):
                maxarea = area
                bestcontour = contour
        if bestcontour is not None:
            moments = cv2.moments( bestcontour, 0 )
            if self.show_images:
                cv2.drawContours( img, [bestcontour], 0, (255,0,0), 2 )
        return moments

    # Read an image from the stream
    # Return the coordinates and area of the object (posx, posy, area)
    # The centre of the image is at (posx, posy) = (0, 0)
    def Track( self, stream ):
        # "Decode" the image from the array, preserving colour
        data = np.fromstring( stream.getvalue(), dtype=np.uint8 )
        img  = cv2.imdecode( data, 1 )
        
        # Camera applies gaussian blur - no need to do it again.
        # img = cv2.smooth( img, cv2.BLUR, 3 )

        # Generate the thresholded image to identify 
        imgFiltered = self._ColorThreshold( img )

        if self.show_images:
            cv2.imshow( "processed", imgFiltered )

        moments = None
        if not self.use_contours:
            # Calculate moments from threshold image
            moments = cv2.moments( imgFiltered, 0 )
        else:
            # Calculate moments from the largest contour
            moments = self._getContourMoments( imgFiltered, img )

        area = 0
        if moments is not None:
            area = moments['m00']
        posX = 0 ; posY = 0
        if(area > 0):
            # Calculating the center position of the object
            posX = int(moments['m10'] / area)
            posY = int(moments['m01'] / area)
            if self.show_images:
                cv2.circle( img, (posX, posY), 20, (0,255,0), 2 );
            # Convert coords so (0,0) is at centre of image and Y is upward
            posX =   posX - img.shape[1]/2
            posY = -(posY - img.shape[0]/2)

        # update video windows
        if self.show_images:
            cv2.imshow( "output", img )

        # Return the coords of the object and it's area
        return (posX, posY, area)

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

    def close():
        # If we were adjusting HSV slice values, print them out
        if self.tune_hsv_thresholds:
            print( "HSV_min = %03d %03d %03d"
                   % (self.HSV_slice[0][0],
                      self.HSV_slice[0][1],
                      self.HSV_slice[0][2]) )
            print( "HSV_max = %03d %03d %03d"
                   % (self.HSV_slice[1][0],
                      self.HSV_slice[1][1],
                      self.HSV_slice[1][2]) )

        # Wind up and shutdown
        cv2.destroyAllWindows()
