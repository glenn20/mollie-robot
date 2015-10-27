// -*- c++ -*-

#ifndef ROBOT_H
#define ROBOT_H

#include <ArduinoJson.h>

#include "Wheel.h"
#include "Head.h"

// A "Robot" is built from a "Head" and two "Wheels"
class Robot {
public:
    Robot(   Wheel&       leftwheel,
	     Wheel&       rightwheel,
	     Head&        head
	);
  
    bool     run( int left, int right );
    bool     setpower( int left, int right );
    void     look( float angleX, float angleY );
    void     look( float angleX );

    void     initialise();
    void     close();
    void     enable();
    void     disable();
    int      leftspeed();
    int      rightspeed();
    int      speed();
    int      direction();

    bool     Loop();

    void     dotrackingPID( int x, int y );
    bool     robotcommand( char* line );

    void     sendjson();
    bool     processjson( char *json );

    Wheel&   leftwheel()  { return m_leftwheel;  };
    Wheel&   rightwheel() { return m_rightwheel; };
    Head&    head()       { return m_head;       };
private:
    Wheel&   m_leftwheel;
    Wheel&   m_rightwheel;
    Head&    m_head;
    bool     m_updated;
};


#endif //ROBOT_H

// Local Variables:
// c-basic-offset: 4
// End: 
