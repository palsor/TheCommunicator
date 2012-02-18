#ifndef CONFIG_H
#define CONFIG_H

// output config
#define SERIAL_RATE 57600
#define COMM_RATE 250

// pins
#define YAW_SERVO_PIN 9
#define PITCH_SERVO_PIN 6
#define ROLL_SERVO_PIN 5
#define THROTTLE_SERVO_PIN 3

// mechanical controls
#define PITCH_CENTER_ANGLE 90  // approximate steering on-center angle for servo
#define PITCH_MECHANICAL_MAX 25  // mechanical limits of servo travel
#define YAW_CENTER_ANGLE 90  // approximate steering on-center angle for servo
#define YAW_MECHANICAL_MAX 25  // mechanical limits of servo travel
#define ROLL_CENTER_ANGLE 90  // approximate steering on-center angle for servo
#define ROLL_MECHANICAL_MAX 25  // mechanical limits of servo travel


#endif
