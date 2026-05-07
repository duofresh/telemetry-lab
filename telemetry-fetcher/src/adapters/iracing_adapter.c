#include "adapters/iracing_shared_memory.h"
#include "rts/telemetry_frame.h"
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>

typedef struct {
    HANDLE hMemMap;
    void* shared_mem;
    irsdk_header* header;
    
    // Offsets to important variables
    int offset_speed;
    int offset_rpm;
    int offset_gear;
    int offset_throttle;
    int offset_brake;
    int offset_steering;
    int offset_lap_dist;
} iracing_adapter_t;

bool iracing_adapter_init(iracing_adapter_t* adapter) {
    adapter->hMemMap = OpenFileMapping(PAGE_READONLY, FALSE, TEXT(IRSDK_MEMMAPFILENAME));
    if (!adapter->hMemMap) return false;
    
    adapter->shared_mem = MapViewOfFile(adapter->hMemMap, FILE_MAP_READ, 0, 0, 0); // map whole file
    if (!adapter->shared_mem) {
        CloseHandle(adapter->hMemMap);
        return false;
    }

    adapter->header = (irsdk_header*)adapter->shared_mem;
    
    // Initialize offsets to -1
    adapter->offset_speed = -1;
    adapter->offset_rpm = -1;
    adapter->offset_gear = -1;
    adapter->offset_throttle = -1;
    adapter->offset_brake = -1;
    adapter->offset_steering = -1;
    adapter->offset_lap_dist = -1;

    // Find variable offsets
    irsdk_varHeader* var_headers = (irsdk_varHeader*)((char*)adapter->shared_mem + adapter->header->varHeaderOffset);
    for (int i = 0; i < adapter->header->numVars; i++) {
        if (strcmp(var_headers[i].name, "Speed") == 0) adapter->offset_speed = var_headers[i].offset;
        else if (strcmp(var_headers[i].name, "RPM") == 0) adapter->offset_rpm = var_headers[i].offset;
        else if (strcmp(var_headers[i].name, "Gear") == 0) adapter->offset_gear = var_headers[i].offset;
        else if (strcmp(var_headers[i].name, "Throttle") == 0) adapter->offset_throttle = var_headers[i].offset;
        else if (strcmp(var_headers[i].name, "Brake") == 0) adapter->offset_brake = var_headers[i].offset;
        else if (strcmp(var_headers[i].name, "SteeringWheelAngle") == 0) adapter->offset_steering = var_headers[i].offset;
        else if (strcmp(var_headers[i].name, "LapDist") == 0) adapter->offset_lap_dist = var_headers[i].offset;
    }

    return true;
}

bool iracing_adapter_read(iracing_adapter_t* adapter, rts_telemetry_frame_t* frame) {
    if (!adapter->header) return false;

    // Get the latest buffer
    int latest = 0;
    for (int i = 1; i < adapter->header->numBuf; i++) {
        if (adapter->header->varBuf[i].tickCount > adapter->header->varBuf[latest].tickCount) {
            latest = i;
        }
    }
    
    char* data_buf = (char*)adapter->shared_mem + adapter->header->varBuf[latest].bufOffset;
    
    // Read variables if offset was found
    if (adapter->offset_speed >= 0) frame->speed_ms = *(float*)(data_buf + adapter->offset_speed);
    if (adapter->offset_rpm >= 0) frame->engine_rpm = *(float*)(data_buf + adapter->offset_rpm);
    if (adapter->offset_gear >= 0) frame->gear = (int8_t)*(int*)(data_buf + adapter->offset_gear);
    if (adapter->offset_throttle >= 0) frame->throttle = *(float*)(data_buf + adapter->offset_throttle);
    if (adapter->offset_brake >= 0) frame->brake = *(float*)(data_buf + adapter->offset_brake);
    if (adapter->offset_steering >= 0) frame->steering = *(float*)(data_buf + adapter->offset_steering);
    if (adapter->offset_lap_dist >= 0) frame->lap_distance = *(float*)(data_buf + adapter->offset_lap_dist);

    frame->session_time_ms = adapter->header->varBuf[latest].tickCount * (1000 / adapter->header->tickRate);

    return true;
}

void iracing_adapter_close(iracing_adapter_t* adapter) {
    if (adapter->shared_mem) UnmapViewOfFile(adapter->shared_mem);
    if (adapter->hMemMap) CloseHandle(adapter->hMemMap);
}

#else
// Stub for non-Windows compilation
typedef struct {
    int dummy;
} iracing_adapter_t;

bool iracing_adapter_init(iracing_adapter_t* adapter) { return false; }
bool iracing_adapter_read(iracing_adapter_t* adapter, rts_telemetry_frame_t* frame) { return false; }
void iracing_adapter_close(iracing_adapter_t* adapter) {}

#endif
