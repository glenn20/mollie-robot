// -*- c++ -*-

#include "Motor-MicroM.h"

int MotorMicroM::m_leftpower = 0;
int MotorMicroM::m_rightpower = 0;

MotorMicroM::MotorMicroM(
  bool     leftmotor
  )
  : m_leftmotor ( leftmotor )
{
}

void MotorMicroM::doinitialise()
{
  if (m_leftmotor) {
    // Only initialise once - ie. for the left motor
    microM.Setup();
  }
}

int MotorMicroM::dosetpower( int power )
{
  int currentpower = (m_leftmotor) ? m_leftpower : m_rightpower;
  if (currentpower == power)
    return power;
  if (m_leftmotor) {
    m_leftpower = power;
  } else {
    m_rightpower = power;
  }
  microM.Motors( m_leftpower,
		 m_rightpower,
		 0, 0 );
  return power;
}

// Local Variables:
// c-basic-offset: 2
// End: 
