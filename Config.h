#ifndef CONFIG_H
#define CONFIG_H

// input config
#define GOOD_CHECKSUM_TIMEOUT 500  // ms

// output config
#define SERIAL_RATE 57600 // baud
#define FAST_SERIAL_XMT_INTERVAL 50  // ms
#define MED_SERIAL_XMT_INTERVAL 500  // ms
#define SLOW_SERIAL_XMT_INTERVAL 5000  // ms

// comm protocols
#define SPI_HEADER 0xAA
#define SPI_FOOTER 0x55

// pins
#define YAW_SERVO_PIN 9
#define PITCH_SERVO_PIN 6
#define ROLL_SERVO_PIN 5
#define THROTTLE_SERVO_PIN 3
#define SPI_SLAVE_ACK_PIN 8
#define SPI_SS_PIN 10

// mechanical controls
#define PITCH_CENTER_ANGLE 90  // approximate steering on-center angle for servo
#define PITCH_MECHANICAL_MAX 25  // mechanical limits of servo travel
#define YAW_CENTER_ANGLE 90  // approximate steering on-center angle for servo
#define YAW_MECHANICAL_MAX 25  // mechanical limits of servo travel
#define ROLL_CENTER_ANGLE 90  // approximate steering on-center angle for servo
#define ROLL_MECHANICAL_MAX 25  // mechanical limits of servo travel


#endif
