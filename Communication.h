#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#define RING_BUFF_SIZE 200

#include <Arduino.h>
#include <SPI.h>

#include "Config.h"
#include "Externs.h"
#include "Structs.h"

class Communication {
  public:
    Communication();
    void init();
    void parseData();
    void sendRadioData();
    void spiInterrupt();
    
  private:
    void resetState();
    void writeByte(byte c);
    
    volatile byte* leadPtr;
    volatile byte ringBuf[RING_BUFF_SIZE];
    volatile boolean leadWrap;
    volatile boolean ringBuffOverflow;
    volatile unsigned long recTimestamp;
    byte* trailPtr;
    
    byte* writePtr;
    byte curStruct;
    byte calcChecksum;
    int state;
    int byteCount;
    int length;
    unsigned long goodChecksums;
    unsigned long badChecksums;
  
    SensorData* sensorDataA;
    NavData* navDataA;
    PilotData* pilotDataA;
};

#endif
