// -*- c++ -*-

#ifndef ROBOT_H
#define ROBOT_H

#include "Wheel.h"
#include "Head.h"
//#include "Json.h"

class RobotState {
public:
    RobotState()
	: head     ( false ),
	  power    ( false ),
	  setspeed ( false ),
	  speed    ( false ),
	  counts   ( false ),
	  pid      ( false ),
	  config   ( false )
	{
    };

public:
    bool head;
    bool power;
    bool setspeed;
    bool speed;
    bool counts;
    bool pid;
    bool config;
};

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

    void     close();
    int      leftspeed();
    int      rightspeed();
    int      speed();
    int      direction();

    bool     Loop();

    void     dotrackingPID( int x, int y );
    bool     robotcommand( char* line );

    void     updatefield();
    void     updatehead();
    void     updatesetspeed();
    void     updatepower();
    void     updatespeed();
    void     updatecounts();
    void     updatepid();

    bool     processtarget( JsonObject& d );
    bool     processjson( const char* json );
    bool     sendstate();

    Wheel&   leftwheel()  { return m_leftwheel;  };
    Wheel&   rightwheel() { return m_rightwheel; };
    Head&    head()       { return m_head;       };
private:
    Wheel&   m_leftwheel;
    Wheel&   m_rightwheel;
    Head&    m_head;
    RobotState m_state;
    bool     m_updated;
};


#endif //ROBOT_H

// Local Variables:
// c-basic-offset: 4
// End: 
