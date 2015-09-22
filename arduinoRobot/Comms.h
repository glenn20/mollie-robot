// -*- c++ -*-

#ifndef COMM_H
#define COMM_H

void SetupComms( int i2c_slave_address );
char *ReadCommand();
void SetupComms_serial( );
char *ReadCommand_serial();

#endif //COMMS_H

// Local Variables:
// c-basic-offset: 4
// End: 
