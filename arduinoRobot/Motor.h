// -*- c++ -*-

#ifndef MOTOR_H
#define MOTOR_H

// A base class for several DC motor control libraries
// The base class does everything except initialise and write to the hardware
// Derived implementations should set doinitialise() and dosetpower()
// This base class is a valid dummy implementation.
class Motor {
public:
    Motor() {};

    void  initialise() {
	doinitialise();
    };
    void  close() {
	setpower( 0 );
    };
    int   setpower( int power ) { // Set the motor power
	if (power == m_power) {
	    return power;
	}
	return m_power = dosetpower( power );
    };
    void  enable() {              // Enable the motor
	if (!m_enabled) {
	    m_enabled = true;
	    setpower( m_power );
	}
    }; 
    void  disable() {             // Disable the motor
	if (m_enabled) {
	    m_enabled = false;
	    setpower( 0 );
	}
    };
    int   power() {               // Return the current power setting
	return m_power;
    };

private:
    // Private virtual method to actually set the motor power
    virtual int  dosetpower( int power ) {
	return power;
    };
    virtual void doinitialise() {
    };

private:
    int      m_power;
    bool     m_enabled;
};

#endif //MOTOR_H

// Local Variables:
// c-basic-offset: 4
// End: 
