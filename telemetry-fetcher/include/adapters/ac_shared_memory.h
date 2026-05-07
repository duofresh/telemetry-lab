#ifndef AC_SHARED_MEMORY_H
#define AC_SHARED_MEMORY_H

#include <stdint.h>

#pragma pack(push, 4)

typedef struct {
    int packetId;
    float gas;
    float brake;
    float up;
    float down;
    float cgHeight;
    float sensorCGHeight;
    float camberRAD[4];
    float damage[4];
    float suspensionTravel[4];
    float drs;
    float tc;
    float heading;
    float pitch;
    float roll;
    float cgMD;
    float drag;
    float powerMD;
    float abs;
    float kersCharge;
    float kersInput;
    int autoShifterOn;
    float rideHeight[2];
    float turboBoost;
    float ballast;
    float airDensity;
    float airTemp;
    float roadTemp;
    float localAngularVel[3];
    float finalFF;
    float performanceMeter;

    int engineBrake;
    int ersRecoveryLevel;
    int ersPowerLevel;
    int ersHeatCharging;
    int ersIsCharging;
    float kersCurrentKJ;

    int drsAvailable;
    int drsEnabled;

    float brakeTemp[4];
    float clutch;

    float tyreCoreTemperature[4];
    float tyreContactPoint[4][3];
    float tyreContactNormal[4][3];
    float tyreContactHeading[4][3];

    float brakeBias;
    float localVelocity[3];

    // Some parts of physics are above, let's keep it simple for what we need.
    // Wait, the official struct is slightly different, but let's just define the bare minimum from the start of the struct, or the correct offsets if we know them.
    // Actually, ACC and AC share a very specific layout for the first ~100 bytes.
} SPageFilePhysics_Partial;

// Actually, it's better to define the exact struct from Assetto Corsa.
typedef struct {
    int packetId;
    float gas;
    float brake;
    float fuel;
    int gear;
    int rpms;
    float steerAngle;
    float speedKmh;
    float velocity[3];
    float accG[3];
    float wheelSlip[4];
    float wheelLoad[4];
    float wheelsPressure[4];
    float wheelAngularSpeed[4];
    float tyreWear[4];
    float tyreDirtyLevel[4];
    float tyreCoreTemperature[4];
    float camberRAD[4];
    float suspensionTravel[4];
    float drs;
    float tc;
    float heading;
    float pitch;
    float roll;
    float cgHeight;
    float carDamage[5];
    int numberOfTyresOut;
    int pitLimiterOn;
    float abs;
    float kersCharge;
    float kersInput;
    int autoShifterOn;
    float rideHeight[2];
    float turboBoost;
    float ballast;
    float airDensity;
    float airTemp;
    float roadTemp;
    float localAngularVel[3];
    float finalFF;
    float performanceMeter;
    int engineBrake;
    int ersRecoveryLevel;
    int ersPowerLevel;
    int ersHeatCharging;
    int ersIsCharging;
    float kersCurrentKJ;
    int drsAvailable;
    int drsEnabled;
    float brakeTemp[4];
    float clutch;
    // ... padding/more fields
} SPageFilePhysics;

typedef struct {
    int packetId;
    int status;
    int session;
    wchar_t currentTime[15];
    wchar_t lastTime[15];
    wchar_t bestTime[15];
    wchar_t split[15];
    int completedLaps;
    int position;
    int iCurrentTime;
    int iLastTime;
    int iBestTime;
    float sessionTimeLeft;
    float distanceTraveled;
    int isInPit;
    int currentSectorIndex;
    int lastSectorTime;
    int numberOfLaps;
    wchar_t tyreCompound[33];
    float replayTimeMultiplier;
    float normalizedCarPosition; // 0.0 to 1.0 lap distance
    int activeCars;
    float carCoordinates[3];     // pos_x, pos_y, pos_z
    int carID;
    int playerCarID;
    float penaltyTime;
    int flag;
    int penalty;
    int idealLineOn;
    int isInPitLane;
    float surfaceGrip;
    int mandatoryPitDone;
    float windSpeed;
    float windDirection;
    // ... more fields
} SPageFileGraphics;

typedef struct {
    wchar_t smVersion[15];
    wchar_t acVersion[15];
    int numberOfSessions;
    int numCars;
    wchar_t carModel[33];
    wchar_t track[33];
    wchar_t playerName[33];
    wchar_t playerSurname[33];
    wchar_t playerNick[33];
    int sectorCount;
    float maxTorque;
    float maxPower;
    int maxRpm;
    float maxFuel;
    float suspensionMaxTravel[4];
    float tyreRadius[4];
    float maxTurboBoost;
    float deprecated_1;
    float deprecated_2;
    int penaltiesEnabled;
    float aidFuelRate;
    float aidTireRate;
    float aidMechanicalDamage;
    int aidAllowTyreBlankets;
    float aidStability;
    int aidAutoClutch;
    int aidAutoBlip;
    // ...
} SPageFileStatic;

#pragma pack(pop)

#endif // AC_SHARED_MEMORY_H
