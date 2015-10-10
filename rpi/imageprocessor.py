"""
Provides the Image class for Glenns Raspberry PI camera capture and image processing.

These are placed on the image processing queue for consumption by the processing
workflows.
"""

import io

# Each ImageProcessor has a stream for capturing images from the camera
# and has a thread for processing it in the run() method.
class Image():
    """
    An object for image capture and processing on the raspberry pi.

    This 

    Methods:
    __init__():  Build the Image record
    reset():     Reset the Image record for re-use
                 (Some workflows may recycle records, rather than create and destroy)

    Attributes:
    stream:      A BytesIO stream for image capture from the PI Camera.
    img:         The OpenCV Image for the processing workflows
    contours:    List of contours for the object identified
    bestcontour: Countour representing the object to be tracked
    time:        Time of image capture
    location:    Tuple of (x,y) for centre of object to be tracked
                 (coords relative to top left of image)
    track:       Tuple of (x,y,area) for object to be tracked
                 (coords relative to centre of image)
    """
    sentinel = object()

    def __init__( self ):
        """
        Instantiate the Image object
        """
        self.stream         = io.BytesIO()
        self.reset()

    def reset( self ):
        self.stream.seek( 0 )
        self.stream.truncate()
        self.img            = None
        self.contours       = None
        self.bestcountour   = None
        self.time           = None
        self.location       = (None, None)
        self.track          = (None, None, None)

# Local Variables:
# python-indent: 4
# tab-width: 4
# indent-tabs-mode: nil
# End: 
