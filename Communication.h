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
    void update();
    void spiInterrupt();
    
  private:
    void parseData();
    void readRadioCmd();
    void sendRadioData();
    void resetState();
    void writeByte(byte c);
    
    volatile byte* leadPtr;
    volatile byte ringBuf[RING_BUFF_SIZE];
    volatile boolean leadWrap;
    byte* trailPtr;
    
    byte* writePtr;
    byte* structEnd;
    byte curStruct;
    byte calcChecksum;
    int state;
    unsigned long commTime;
    unsigned long goodChecksums;
    unsigned long badChecksums;
    int radioCmdStruct, radioCmdOffset, radioCmdMult, radioCmdType;
    int tmpCmdStruct, tmpCmdOffset, tmpCmdMult, tmpCmdType;
    int cmdState;
  
    SensorData* sensorDataA;
    NavData* navDataA;
    PilotData* pilotDataA;
    ErrorData* errorDataA;
    DebugData* debugDataA;
};

#endif
