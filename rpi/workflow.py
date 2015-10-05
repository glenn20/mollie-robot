import io
import time
import threading
import Queue

class Worker( threading.Thread ):
    """
    Hold information for a worker thread
    """
    sentinel = object()

    def __init__( self, processor, inputqueue, outputqueue, otherqueues = None ):
        super( Worker, self ).__init__()
        self.processor      = processor
        self.inputqueue     = inputqueue
        self.outputqueue    = outputqueue
        self.otherqueues    = otherqueues
        self.count          = 0
        self.start()

    def run( self ):
        """
        Extract items off input queue, process, and then put on the output queue.
        Args:
            inputqueue:   Queue.Queue
            outputqueues: A list/tuple of Queue.Queue-s
            processor:    Function to do the work - process the items.
        """
        while True:
           # Wait for an item to appear on the processing queue
            item = self.inputqueue.get()
            # If this is a sentinel - put it back on the queue and end this worker thread.
            #  - need to put it back so other threads will also find it and terminate.
            if (item is self.sentinel):
                self.inputqueue.put( self.sentinel )
                # Also pass the sentinel down the processing pipeline
                if (self.outputqueue is not None):
                    self.outputqueue.put( self.sentinel )
                break
            try:
                # Do the processing
                if self.otherqueues is not None:
                    item = self.processor( item, self.otherqueues )
                else:
                    item = self.processor( item )
                self.count += 1
            finally:
                # Put the item on the output queue
                if (self.outputqueue is not None):
                    self.outputqueue.put( item )

    def close( self ):
        # Put a sentinel on the end of the work queue
        # - Worker thread will close when it is received.
        self.inputqueue.put( self.sentinel )

class WorkerPool():
    """
    A pool of threads to process items off a queue...
    """
    def __init__( self, numberofworkers, processor, inputqueue, outputqueue, otherqueues = None ):
        """
        Args:
        - numberofworkers:  The number of worker threads to create.
        - processor:        The function to process items: first arg is the item to process
        -                   An optional second arg which is additional queues for other output
        - inputqueue:       The queue for receiving items for processing
        - outputqueue:      The queue to put processed items on when finished
        - otherqueues:      An optional list/tuple of queues to be passed to the processor function
        -                   The procesor function may use these to run new workflows
        """
        self.numberofworkers = numberofworkers
        self.processor       = processor
        self.inputqueue      = inputqueue
        self.outputqueue     = outputqueue
        self.otherqueues     = otherqueues
        self.workers         = []

        # Now create the Worker threads
        self.addworkers( numberofworkers )

    def addworkers( self, numberofworkers ):
        """
        Add more worker threads to this pool.
        Args:
        - numberofworkers: The number of worker threads to add.
        """
        for i in range( numberofworkers ):
            self.workers.append( Worker( self.processor, self.inputqueue,
                                         self.outputqueue, self.otherqueues ) )

    def count( self ):
        """
        Get the number of items processed by all the workers in this pool.
        """
        return sum( (worker.count for worker in self.workers) )

    def close( self ):
        """
        Flag all the worker threads to finish processing the queues and close down.
        """
        for worker in self.workers:
            worker.close()
        for worker in self.workers:
            worker.join()

class WorkflowManager():
    """
    Manage the queues and threads for workflow.
    """
    def __init__( self, workflow ):
        """
        Takes a workflow description and builds and initiates tasks to process the workflow
        Args:
        workflow: A list/tuple of WorkerPool objects which comprise the workflow
        """
        self.starttime  = time.time()
        self.finishtime = None
        self.workflow   = workflow

    def count( self ):
        # Return the number of items processed
        # - only count the items processed by the first pool of workers.
        return self.workflow[0].count() if self.workflow is not None else 0

    def report( self ):
        """
        Print a summary of the workflow statistics.
        """
        elapsedtime = self.finishtime - self.starttime
        count = self.count()
        print( "\r\n" )
        print( 'Processed %d items in %d seconds at %.2ffps\r\n'
               % ( count, elapsedtime, count / elapsedtime ) )

    # Cleanup up all the worker threads
    def close( self ):
        """
        Close down the workflow processing threads.
        """
        # Tell the worker threads to finish and wait for them to terminate...
        for workerpool in self.workflow:
            workerpool.close()
        self.finishtime = time.time()
        self.report()

    # So we can use this class as a ContextManager
    def __enter__( self ):
        return self

    def __exit__( self, exc_type, exc_val, exc_tb ):
        self.close()

# Local Variables:
# python-indent: 4
# tab-width: 4
# indent-tabs-mode: nil
# End: 
