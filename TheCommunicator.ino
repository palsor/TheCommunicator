#include <SPI.h>
#include <Servo.h>

#include "pins_arduino.h"

#include "Structs.h"
#include "Communication.h"
#include "Controller.h"

Communication comms;
Controller controller;

SensorData* sensorPtr;
NavData* navPtr;
PilotData* pilotPtr;

void setup()
{
  comms.init();
  controller.init();
  
  // setup spi 
  pinMode(MISO, OUTPUT); // have to send on master in, *slave out*
  SPCR |= _BV(SPE); // turn on SPI in slave mode
  SPCR |= _BV(SPIE); // now turn on interrupts
  pinMode(SPI_SLAVE_ACK_PIN, OUTPUT);  // ack pin back to master
  pinMode(SPI_SS_PIN, INPUT);  // spi source select input
  digitalWrite(SPI_SLAVE_ACK_PIN, HIGH);  // initial state for ack (toggle based)
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
  comms.parseData();
  controller.update();
  comms.sendRadioData();
} 
