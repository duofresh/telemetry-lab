#ifndef AC_ADAPTER_H
#define AC_ADAPTER_H

#include "adapters/ac_shared_memory.h"
#include "rts/telemetry_frame.h"
#include <stdbool.h>

#ifdef _WIN32
#include <windows.h>

typedef struct {
    HANDLE hPhysics;
    HANDLE hGraphics;
    HANDLE hStatic;
    
    SPageFilePhysics* physics;
    SPageFileGraphics* graphics;
    SPageFileStatic* static_info;
} ac_adapter_t;

#else
// Stub for non-Windows compilation
typedef struct {
    int dummy;
} ac_adapter_t;
#endif

bool ac_adapter_init(ac_adapter_t* adapter);
bool ac_adapter_read(ac_adapter_t* adapter, rts_telemetry_frame_t* frame);
void ac_adapter_close(ac_adapter_t* adapter);

#endif // AC_ADAPTER_H
