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

//  yawServo.attach(YAW_SERVO_PIN);
//  yawServo.write(YAW_CENTER_ANGLE);  // init servo to mechanical center

//  rollServo.attach(ROLL_SERVO_PIN);
//  rollServo.write(0);  // init servo to 0

} 


//
// send new signals to servos and motor
//
void Controller::update() {
  applyThrottleValue();
  applyPitchValue();
  applyYawValue();
  applyRollValue();
} 


//
// applies throttleValue setting to throttle control hardware
//
void Controller::applyThrottleValue() {
//  throttleServo.write(map(pilotData.throttleValue,0,99,0,359));
}


//
// applies pitchValue setting to pitch control hardware
//
void Controller::applyPitchValue() {
//  pitchServo.write(pilotData.pitchValue);
}


//
// applies yawValue setting to yaw control hardware
//
void Controller::applyYawValue() {
  //yawServo.write(pilotData.yawValue);
}


//
// applies rollValue setting to roll control hardware
//
void Controller::applyRollValue() {
//  rollServo.write(pilotData.rollValue);
}
