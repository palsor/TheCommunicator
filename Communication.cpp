#include "Communication.h"

//
// constructor
// 
Communication::Communication() {}

//
// init
//
void Communication::init() {
  
  commDataA = (CommData*)malloc(sizeof(CommData));
  commDataB = (CommData*)malloc(sizeof(CommData));
  
  Serial.begin(SERIAL_RATE);
  curStruct = commDataA;
  resetState();
  
  goodChecksums = 0;
  badChecksums = 0;
  commTime = 0;
  
  Serial.println("starting...");
}

//
// update - parse data input from SPI and send data output via radio
//
void Communication::update() {
  
  // send data out via radio
  unsigned long curTime = millis();
  if (curTime > commTime + COMM_RATE) {
    commTime = curTime;
    Serial.print("Good: ");
    Serial.print(goodChecksums);
    Serial.print(", Bad: ");
    Serial.println(badChecksums);
  }
}

//
//
// 
void Communication::spiInterrupt() {
  byte c = SPDR;  // grab byte from SPI Data Register
  
  //
  // state machine for filling in data
  // 
  // state 0 = waiting to begin
  // state 1 = got first 0xAA
  // state 2 = got second 0xAA, now filling in data
  // state 3 = got first 0x55, waiting to see if we get a second
  // state 4 = got second 0x55, validating checksum
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
    if (c == 0x55)
      state = 3;
    else {
      *ptr = c;
      ptr += 1;
      calcChecksum += c;
    }
  } 
  else if (state == 3) {
    if (c == 0x55) {
      state = 4;
    }
    else {
      *ptr = 0x55;
      ptr += 1;
      calcChecksum += 0x55;
      *ptr = c;
      ptr += 1;
      calcChecksum += c;
      state = 2;
    }
  } 
  else if (state == 4)
  {    
    if (c == calcChecksum) {
      // good checksum, flip pointers around
      sensorPtr = &(curStruct->sensorData);
      navPtr = &(curStruct->navData);
      pilotPtr = &(curStruct->pilotData);
      errorPtr = &(curStruct->errorData);
      debugPtr = &(curStruct->debugData);
      
      if (curStruct == commDataA) {
        curStruct = commDataB;
      } else {
        curStruct = commDataA;
      }
      
      goodChecksums++;
    } else {
      badChecksums++;  
    }
    
    // regardless of whether the checksum was good, we have to reset everything
    resetState();
  }
  
  if (ptr >= (byte*)curStruct + sizeof(CommData)) {
    resetState();
  }
}

void Communication::resetState() {
  ptr = (byte*)curStruct;
  calcChecksum = 0;
  state = 0;
}
