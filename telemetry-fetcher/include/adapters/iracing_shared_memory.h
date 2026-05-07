#ifndef IRACING_SHARED_MEMORY_H
#define IRACING_SHARED_MEMORY_H

#include <stdint.h>

#pragma pack(push, 4)

#define IRSDK_MEMMAPFILENAME "Local\\IRSDKMemMapFileName"

// iRacing Shared Memory Header
typedef struct {
    int ver;             // api header version
    int status;          // bitfield
    int tickRate;        // ticks per second (60 or 360 etc)

    // session information, updated infrequently
    int sessionInfoUpdate; // incremented when session info changes
    int sessionInfoLen;    // length in bytes of session info string
    int sessionInfoOffset; // offset from header to session info string

    // State data, multiple buffers are used to avoid tearing
    int numVars;         // length of array pointed to by varHeaderOffset
    int varHeaderOffset; // offset to irsdk_varHeader[numVars] array

    int numBuf;          // <= 4 buffers (usually 3)
    int bufLen;          // length in bytes for one line
    int pad[2];

    struct {
        int tickCount;   // used to detect changes in data
        int bufOffset;   // offset from header to buffer
        int pad[2];
    } varBuf[4];
} irsdk_header;

// iRacing Variable Header
typedef struct {
    int type;            // irsdk_VarType
    int offset;          // offset from start of buffer row
    int count;           // number of entrys (array)
    int countAsTime;
    char pad[3];         // (16 byte align)
    char name[32];       // variable name
    char desc[64];       // description
    char unit[32];       // units
} irsdk_varHeader;

#pragma pack(pop)

#endif // IRACING_SHARED_MEMORY_H
