#ifndef STRUCTS_H
#define STRUCTS_H

#define SENSOR_DATA 0
#define NAV_DATA 1
#define ERROR_DATA 2
#define DEBUG_DATA 3
#define PILOT_DATA 4
#define CAPT_DATA 5

struct Waypoint {
  float latitude;
  float longitude;
} __attribute__((packed));

struct Vector {
  float direction;
  float magnitude;
} __attribute__((packed));

struct SensorData {
  float magBearing;     // degrees  
  float gpsBearing;     // degrees
  float gpsSpeed;       // m/s       
  Waypoint curLocation; // lat/long in degrees
  float gpsHDOP;        // decimal
  float gpsAltitude;    // meters
  float pitch;          // degrees in earth's frame of reference
  float yaw;            // degrees in earth's frame of reference
  float roll;           // degrees in earth's frame of reference
  float pressAltitude;  // meters
  float airspeed[3];    // m/s
  float battVoltage;    // volts
  bool gpsUpdated;      // 1 = GPS data was updated, 0 = GPS was not updated
  byte radioMuxSelect;  // 1 = radio, 0 = autopilot
  byte gpsSatellites;   // number of satellites
  byte gpsFixType;      // 1 = no fix, 2 = 2D, 3 = 3D
} __attribute__((packed));

struct CaptData {
  int curState;  // current state of navigation state machine
  int prevState;  // previous state of navigation state machine
  unsigned long lastStateTransitionTime;  // last navigator state transition time
} __attribute__((packed));

struct NavData {
  int maxValidCourseIdx;  // max valid index of waypoints & courseDistances    
  unsigned long lastUpdateTime;  // last navigator update loop time
  Waypoint estLocation;  // estimated location updated between GPS fixes
  Vector curDistance;  // calculated distance/bearing to next waypoint
  Vector estDistance;  // estimated distance since last loop iteration (NOT to next waypoint)
  Vector curGroundSpeed;  // created from gpsSpeed/gpsBearing
  Vector estGroundSpeed;  // created from curAirSpeed+curWindSpeed
  Vector curAirSpeed;  // created from airspeed/magBearing
  Vector curWindSpeed;  // calculated from curGroundSpeed - curAirSpeed
  Vector targetAirSpeed;  // calculated desired heading in plane reference with normalized speed
} __attribute__((packed));

struct PilotData {
  float throttleValue;  // throttle value sent to controller (0-99%)
  float pitchValue;  // pitch value sent to controller (0-360 degrees)
  float yawValue;  // yaw value sent to controller (0-360 degrees)
  float rollValue;  // roll value sent to controller (0-360 degrees)
} __attribute__((packed));

struct ErrorData {
  bool compassReadError;
  bool navWaypointError;
} __attribute__((packed));

struct DebugData {
  boolean linkTestSuccess;
  unsigned long gpsParseErrors;
  unsigned long gpsSentences;
  unsigned long mainLoopIterations;
  unsigned long sensorUpdates;
  unsigned long navUpdates;
  unsigned long sensorAvgDelayTime;
  unsigned long sensorAvgRunTime;
  unsigned long sensorWorstCaseDelayTime;
  unsigned long sensorWorstCaseRunTime;
  unsigned long navAvgDelayTime;
  unsigned long navAvgRunTime;
  unsigned long navWorstCaseDelayTime;
  unsigned long navWorstCaseRunTime;
  unsigned long averageSerialTime;
  unsigned long spiXmtCount;
  unsigned long spiXmtErrorCount;
} __attribute__((packed));

#endif
