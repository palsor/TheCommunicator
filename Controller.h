#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <Arduino.h>
#include <Servo.h>
#include "Config.h"
#include "Externs.h"

class Controller {
  public:
    Controller();
    void init();  // creates hardware control objects and initializes to safe values
    void update();  // send new signals to servos and motor
    
  private:
    Servo throttleServo;
    Servo pitchServo;
    Servo yawServo;
    Servo rollServo;
};

#endif
