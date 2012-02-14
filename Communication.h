#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include <Arduino.h>
#include <SPI.h>

#include "Config.h"
#include "Externs.h"
#include "Structs.h"

struct CommData {
  SensorData sensorData;
  NavData navData;
  PilotData pilotData;
  ErrorData errorData;
  DebugData debugData;
} __attribute__((packed));

class Communication {
  public:
    Communication();
    void init();
    void update();
    void spiInterrupt();
    
  private:
    volatile CommData* curStruct;
    volatile byte* ptr;
    volatile byte calcChecksum;
    volatile int state;
    volatile unsigned long goodChecksums;
    volatile unsigned long badChecksums;
    
    unsigned long commTime;
  
    CommData* commDataA;
    CommData* commDataB;
    
    void resetState();
};

#endif
