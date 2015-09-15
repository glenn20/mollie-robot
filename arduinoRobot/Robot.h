// -*- c++ -*-

#ifndef ROBOT_H
#define ROBOT_H

#include "Wheel.h"
#include "Head.h"

// A "Robot" is built from a "Head" and two "Wheels"
class Robot {
public:
    Robot(   Wheel&       leftwheel,
	     Wheel&       rightwheel,
	     Head&        head
	);
  
    bool     run( int speed, int direction );
    void     look( float angleX, float angleY );
    void     look( float angleX );

    void     initialise();
    void     enable();
    void     disable();
    int      leftspeed();
    int      rightspeed();
    int      speed();
    int      direction();

    bool     Loop();

    Wheel&   leftwheel()  { return m_leftwheel;  };
    Wheel&   rightwheel() { return m_rightwheel; };
    Head&    head()       { return m_head;       };
private:
    Wheel&   m_leftwheel;
    Wheel&   m_rightwheel;
    Head&    m_head;
    int      m_leftspeed;
    int      m_rightspeed;
    int      m_speed;
    int      m_direction;
};


#endif //ROBOT_H

// Local Variables:
// c-basic-offset: 4
// End: 
