#include "Communication.h"

//
// constructor
// 
Communication::Communication() {}

//
// init
//
void Communication::init() {
  
  sensorDataA = (SensorData*)malloc(sizeof(SensorData));
  navDataA = (NavData*)malloc(sizeof(NavData));
  pilotDataA = (PilotData*)malloc(sizeof(PilotData));
    
  sensorPtr = (SensorData*)malloc(sizeof(SensorData));
  navPtr = (NavData*)malloc(sizeof(NavData));
  pilotPtr = (PilotData*)malloc(sizeof(PilotData));
  
  leadPtr = ringBuf;
  trailPtr = (byte*)ringBuf;
  leadWrap = false;
  ringBuffOverflow = false;
  
  state = 0;
  calcChecksum = 0;
  byteCount = 0;
  length = 0;
  
  Serial.begin(SERIAL_RATE);
}

//
// spiInterrupt - called when an SPI character shows up
// 
void Communication::spiInterrupt() {
  // add a new character into the ring buffer
  *leadPtr = SPDR;
  
  // move the lead pointer
  if (leadPtr < ringBuf + RING_BUFF_SIZE - 1)
    leadPtr++;
  else {
    leadPtr = ringBuf;
    leadWrap = true;
  }
  
  // check to see if we caught up with the trailing ptr
  if (leadPtr == trailPtr)
    ringBuffOverflow = true;
}
  
//
// parseData - parses any new data in the ring buffer
//
void Communication::parseData() {
  
  while ((trailPtr < leadPtr) || leadWrap) {
  
    byte c = *trailPtr;
    if (trailPtr < ringBuf + RING_BUFF_SIZE - 1)
      trailPtr++;
    else {
      trailPtr = (byte*)ringBuf;
      leadWrap = false;
    }
    
    //
    // state machine for filling in data
    // 
    // state 0 = waiting to begin
    // state 1 = got first 0xAA
    // state 2 = got second 0xAA, determine which struct we're fillig in
    // state 3 = now filling in data
    // state 4 = got first 0x55, waiting to see if we get a second
    // state 5 = got second 0x55, validating checksum
    //
   
    if (state == 0) {
      if (c == 0xAA)
        state = 1;
    } 
    else if (state == 1) {
      if (c == 0xAA)
        state = 2;
      else
        state = 0;
    } 
    else if (state == 2) {
      state = 3;
      curStruct = c;
      if (c == SENSOR_DATA) {
        writePtr = (byte*)sensorDataA;
        length = sizeof(SensorData);
      }
      else if (c == NAV_DATA) {
        writePtr = (byte*)navDataA;
        length = sizeof(NavData);
      }
      else if (c == PILOT_DATA) {
        writePtr = (byte*)pilotDataA;
        length = sizeof(PilotData);
      }
      else {
        state = 0; // something went wrong, reset
      }  
    }
    else if (state == 3) {
      if (c == 0x55)
        state = 4;
      else {
        writeByte(c);
      }
    } 
    else if (state == 4) {
      if (c == 0x55) {
        state = 5;
      }
      else {
        writeByte(0x55);
        writeByte(c);
        state = 3;
      }
    } 
    else if (state == 5)
    {    
      void* tempPtr;
      if ((c == calcChecksum) && (byteCount == length)) {
        // good checksum, flip pointers around
        if (curStruct == SENSOR_DATA) {
          tempPtr = (void*)sensorPtr;
          sensorPtr = sensorDataA;
          sensorDataA = (SensorData*)tempPtr;
        } else if (curStruct == NAV_DATA) {
          tempPtr = (void*)navPtr;
          navPtr = navDataA;
          navDataA = (NavData*)tempPtr;
        } else if (curStruct == PILOT_DATA) {
          tempPtr = (void*)pilotPtr;
          pilotPtr = pilotDataA;
          pilotDataA = (PilotData*)tempPtr;
        }
        goodChecksums++;
      } else {
        badChecksums++;  
      }
      
      // regardless of whether the checksum was good, we have to reset everything
      resetState();
    }
  }
} 
  
void Communication::writeByte(byte c) {
  if (byteCount <= length) {
    *(writePtr + byteCount) = c;
    calcChecksum += c;
    byteCount++;
  } else {
    resetState();
  }
}
  
void Communication::resetState() {
  calcChecksum = 0;
  state = 0;  
  byteCount = 0;
}

//
// sendRadioData - sends data out through the radio
//
void Communication::sendRadioData () {
  
}
