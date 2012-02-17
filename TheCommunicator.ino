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
  controller.init();
  
  // setup spi 
  pinMode(MISO, OUTPUT); // have to send on master in, *slave out*
  SPCR |= _BV(SPE); // turn on SPI in slave mode
  SPCR |= _BV(SPIE); // now turn on interrupts
} 

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
  controller.update();
} 
