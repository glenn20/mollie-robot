// -*- c++ -*-

#ifndef MOTOR_MICROM_H
#define MOTOR_MICROM_H

#include <microM.h>

#include "Motor.h"

class MotorMicroM: public Motor {
public:
  MotorMicroM( bool leftmotor );

private:
  void       doinitialise();

  int        dosetpower( int power );  // Set the wheel power (-255 to 255)

private:
  bool       m_leftmotor;
  static int m_leftpower;
  static int m_rightpower;
};

#endif //MOTOR_MICROM_H

// Local Variables:
// c-basic-offset: 2
// End: 
