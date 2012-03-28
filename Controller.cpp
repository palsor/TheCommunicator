#include "Controller.h"

//
// constructor
//
Controller::Controller() {}


//
// creates hardware control objects and initializes to safe values
//
void Controller::init() {
  lastThrottleValue = -1;
  lastPitchValue = -1;
  lastYawValue = -1;
  lastRollValue = -1;
  
  throttleServo.attach(THROTTLE_SERVO_PIN);
  throttleServo.write(0);  // init servo to 0

  pitchServo.attach(PITCH_SERVO_PIN);
  pitchServo.write(PITCH_CENTER_ANGLE);  // init servo to 0

  yawServo.attach(YAW_SERVO_PIN);
  yawServo.write(YAW_CENTER_ANGLE);  // init servo to mechanical center

  rollServo.attach(ROLL_SERVO_PIN);
  rollServo.write(ROLL_CENTER_ANGLE);  // init servo to 0
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
  throttleServo.write(map(pilotPtr->throttleValue,0,99,0,359));
}


//
// applies elevatorAngle setting to pitch control hardware
//
void Controller::applyPitchValue() {
  int angle = round(pilotPtr->elevatorAngle);
  if(angle != lastPitchValue) { 
    pitchServo.write(angle);
    lastPitchValue = angle;
  }
}


//
// applies rudderAngle setting to yaw control hardware
//
void Controller::applyYawValue() {
  int angle = round(pilotPtr->rudderAngle);
  if(angle != lastYawValue) { 
    yawServo.write(angle);
    lastYawValue = angle;
  }
}


//
// applies aileronAngle setting to roll control hardware
//
void Controller::applyRollValue() {
  int angle = round(pilotPtr->aileronAngle);
  if(angle != lastRollValue) { 
    rollServo.write(angle);
    lastRollValue = angle;
  }
}

