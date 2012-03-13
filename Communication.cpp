#include "Communication.h"

//
// constructor
// 
Communication::Communication() {}

//
// init
//
void Communication::init() {
  ackPinState = true;
  slowStructToTrans = SENSOR_DATA;
  medStructToTrans = DEBUG_DATA;
  
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
  
  lastFastXmtTime = millis();
  lastMedXmtTime = millis();
  lastSlowXmtTime = millis();
  
  Serial.begin(SERIAL_RATE);
}

//
// spiInterrupt - called when an SPI character shows up
// 
void Communication::spiInterrupt() {
  // add a new character into the ring buffer
  *leadPtr = SPDR;

  // toggle ack pin
  if(ackPinState == true) {
    digitalWrite(SPI_SLAVE_ACK_PIN, LOW);
    ackPinState = false;
  } else {  
    digitalWrite(SPI_SLAVE_ACK_PIN, HIGH);
    ackPinState = true;
  }
  
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
      if (c == SPI_HEADER)
        state = 1;
    } 
    else if (state == 1) {
      if (c == SPI_HEADER)
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
      if (c == SPI_FOOTER)
        state = 4;
      else {
        writeByte(c);
      }
    } 
    else if (state == 4) {
      if (c == SPI_FOOTER) {
        state = 5;
      }
      else {
        writeByte(SPI_FOOTER);
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
  unsigned long curTime = millis();


  // Fast throttled transmit loop
  if (curTime - lastFastXmtTime > FAST_SERIAL_XMT_INTERVAL) {
    lastFastXmtTime = curTime;
  
    // fast xmt data here
//    transmitStruct(PILOT_DATA, (byte*)pilotPtr, sizeof(PilotData));
  }
  
  // Medium throttled transmit loop
  if (curTime - lastMedXmtTime > MED_SERIAL_XMT_INTERVAL) {
    lastMedXmtTime = curTime;
      
    // medium xmt data here
    switch(medStructToTrans) {

      case SENSOR_DATA:
//        transmitStruct(SENSOR_DATA, (byte*)sensorPtr, sizeof(SensorData));
        Serial.println(sensorPtr->yaw,DEC);
        medStructToTrans = NAV_DATA;
        break;
      
      case NAV_DATA:
//        transmitStruct(NAV_DATA, (byte*)navPtr, sizeof(NavData));
        medStructToTrans = SENSOR_DATA;
        break;
      
      default:
        medStructToTrans = SENSOR_DATA;
        break;
    }
  }  
  
  // Slow throttled transmit loop
//  if (curTime - lastSlowXmtTime > SLOW_SERIAL_XMT_INTERVAL) {
//    lastSlowXmtTime = curTime;
//      
//    // slow xmt data here
//    switch(slowStructToTrans) {
//
//      case DEBUG_DATA:
//        transmitStruct(DEBUG_DATA, (byte*)debugPtr, sizeof(DebugData));
//        slowStructToTrans = ERROR_DATA;
//        break;
//      
//      case ERROR_DATA:
//        transmitStruct(ERROR_DATA, (byte*)errorPtr, sizeof(ErrorData));
//        slowStructToTrans = DEBUG_DATA;
//        break;
//      
//      default:
//        slowStructToTrans = DEBUG_DATA;
//        break;
//    }
//  }  
}

//
// transmitStruct - sends a whole struct to the radio
//
void Communication::transmitStruct(byte id, byte* ptr, int length) {
  Serial.print(id,DEC);
  for (byte* temp = ptr; temp < ptr + length; temp++) {
    Serial.print(*temp,HEX);
  }
  Serial.print("\n");
}
