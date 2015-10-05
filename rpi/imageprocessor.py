import io

# Each ImageProcessor has a stream for capturing images from the camera
# and has a thread for processing it in the run() method.
class Image():
    """
    Image record - .

    This is a helper class for the ProcessorManager class

    Methods:
    __init__(): Build the Image record
    reset():    Reset the Image record for re-use
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
