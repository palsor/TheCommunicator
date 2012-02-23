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
  errorDataA = (ErrorData*)malloc(sizeof(ErrorData));
  debugDataA = (DebugData*)malloc(sizeof(DebugData));
    
  sensorPtr = (SensorData*)malloc(sizeof(SensorData));
  navPtr = (NavData*)malloc(sizeof(NavData));
  pilotPtr = (PilotData*)malloc(sizeof(PilotData));
  errorPtr = (ErrorData*)malloc(sizeof(ErrorData));
  debugPtr = (DebugData*)malloc(sizeof(DebugData));
  
  leadPtr = ringBuf;
  trailPtr = (byte*)ringBuf;
  leadWrap = false;
  
  state = 0;
  calcChecksum = 0;
  byteCount = 0;
  length = 0;
  
  commTime = 0; 
  radioCmdStruct = 0;
  radioCmdOffset = 0;
  radioCmdMult = 0;
  radioCmdType = 4;
  cmdState = 0;
  
  
  Serial.begin(SERIAL_RATE);
}

//
// update - parse data input from SPI and send data output via radio
//
void Communication::update() {
  // parse the data in the ring buffer from SPI
  parseData();
  // send data out via radio
  readRadioCmd();
  sendRadioData();
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
        structEnd = (byte*)sensorDataA + length;
      }
      else if (c == NAV_DATA) {
        writePtr = (byte*)navDataA;
        length = sizeof(NavData);
        structEnd = (byte*)navDataA + length;
      }
      else if (c == PILOT_DATA) {
        writePtr = (byte*)pilotDataA;
        length = sizeof(PilotData);
        structEnd = (byte*)pilotDataA + length;
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
  if (writePtr < structEnd) {
    *writePtr = c;
    writePtr++;
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
// readRadioCmd - reads any data from the radio and parses it
//
void Communication::readRadioCmd() {
  while (Serial.available()) {
    char c = Serial.read();
    
    if (cmdState == 0) {
      tmpCmdStruct = c - 48;
      cmdState = 1;
    } else if (cmdState == 1) {
      tmpCmdOffset = c - 48;
      cmdState = 2;
    } else if (cmdState == 2) {
      tmpCmdMult = c - 48;
      cmdState = 3;
    } else if (cmdState == 3) {
      tmpCmdType = c - 48;
      cmdState = 0;
      
      radioCmdStruct = tmpCmdStruct;
      radioCmdOffset = tmpCmdOffset;
      radioCmdMult = tmpCmdMult;
      radioCmdType = tmpCmdType;  
    }
  }
}

//
// sendRadioData - sends data out through the radio
//
void Communication::sendRadioData () {
  
  unsigned long curTime = millis();
  if (curTime > commTime + COMM_RATE) {
    commTime = curTime;
    
    // print stuff here
    byte* tempPtr;
    
    if (radioCmdStruct == SENSOR_DATA) {
          tempPtr = (byte*)sensorPtr;
    } else if (radioCmdStruct == NAV_DATA) {
          tempPtr = (byte*)navPtr;
    } else if (radioCmdStruct == ERROR_DATA) {
          tempPtr = (byte*)errorPtr;
    } else if (radioCmdStruct == DEBUG_DATA) {
          tempPtr = (byte*)debugPtr;
    } else if (radioCmdStruct == PILOT_DATA) {
          tempPtr = (byte*)pilotPtr;
    }
    
    tempPtr += radioCmdOffset * radioCmdMult;
    
    if (radioCmdType == 1) {
      Serial.println(*(boolean*)tempPtr);  
    } else if (radioCmdType == 2) {
      Serial.println(*(int*)tempPtr); 
    } else if (radioCmdType == 3) {
      Serial.println(*(unsigned long*)tempPtr); 
    } else if (radioCmdType == 4) {
      Serial.println(*(float*)tempPtr, DEC); 
    }
  }
}
