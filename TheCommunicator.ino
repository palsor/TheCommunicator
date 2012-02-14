#include <SPI.h>
#include <Servo.h>

#include "pins_arduino.h"

#include "Structs.h"
#include "Communication.h"
#include "Controller.h"

Communication comms;
Controller controller;

volatile SensorData* sensorPtr;
volatile NavData* navPtr;
volatile PilotData* pilotPtr;
volatile ErrorData* errorPtr;
volatile DebugData* debugPtr;

void setup()
{
  comms.init();
  //controller.init();
  
  // have to send on master in, *slave out*
  pinMode(MISO, OUTPUT);
  
  // turn on SPI in slave mode
  SPCR |= _BV(SPE);

  // now turn on interrupts
  SPCR |= _BV(SPIE); 
}  // end of setup

//
// SPI interrupt routine
//
ISR (SPI_STC_vect)
{
  comms.spiInterrupt();
} 

// main loop - wait for flag set in interrupt routine
void loop()
{
  comms.update();
  //controller.update();
} 
