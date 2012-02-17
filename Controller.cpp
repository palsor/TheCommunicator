#include "Controller.h"

//
// constructor
//
Controller::Controller() {}


//
// creates hardware control objects and initializes to safe values
//
void Controller::init() {

//  throttleServo.attach(THROTTLE_SERVO_PIN);
//  throttleServo.write(0);  // init servo to 0

//  pitchServo.attach(PITCH_SERVO_PIN);
//  pitchServo.write(0);  // init servo to 0

  yawServo.attach(YAW_SERVO_PIN);
  yawServo.write(YAW_CENTER_ANGLE);  // init servo to mechanical center

//  rollServo.attach(ROLL_SERVO_PIN);
//  rollServo.write(0);  // init servo to 0

} 


//
// send new signals to servos and motor
//
void Controller::update() {
  //throttleServo.write(map(pilotPtr->throttleValue, 0, 99, 0, 359));
  //pitchServo.write(pilotPtr->pitchValue);
  yawServo.write(pilotPtr->yawValue);
  //rollServo.write(pilotPtr->rollValue);
} 
