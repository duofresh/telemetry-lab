#include "adapters/ac_shared_memory.h"
#include "rts/telemetry_frame.h"
#include <stdio.h>
#include <stdbool.h>

#include "adapters/ac_adapter.h"

#ifdef _WIN32

bool ac_adapter_init(ac_adapter_t* adapter) {
    adapter->hPhysics = OpenFileMapping(FILE_MAP_READ, FALSE, TEXT("Local\\acpmf_physics"));
    if (!adapter->hPhysics) return false;
    adapter->physics = (SPageFilePhysics*)MapViewOfFile(adapter->hPhysics, FILE_MAP_READ, 0, 0, 0);

    adapter->hGraphics = OpenFileMapping(FILE_MAP_READ, FALSE, TEXT("Local\\acpmf_graphics"));
    if (!adapter->hGraphics) return false;
    adapter->graphics = (SPageFileGraphics*)MapViewOfFile(adapter->hGraphics, FILE_MAP_READ, 0, 0, 0);

    adapter->hStatic = OpenFileMapping(FILE_MAP_READ, FALSE, TEXT("Local\\acpmf_static"));
    if (!adapter->hStatic) return false;
    adapter->static_info = (SPageFileStatic*)MapViewOfFile(adapter->hStatic, FILE_MAP_READ, 0, 0, 0);

    return (adapter->physics && adapter->graphics && adapter->static_info);
}

bool ac_adapter_read(ac_adapter_t* adapter, rts_telemetry_frame_t* frame) {
    if (!adapter->physics || !adapter->graphics) return false;

    // We assume the game is updating these structs live. We just sample them.
    frame->session_time_ms = (uint32_t)(adapter->graphics->iCurrentTime);
    frame->throttle = adapter->physics->gas;
    frame->brake = adapter->physics->brake;
    frame->clutch = adapter->physics->clutch;
    frame->steering = adapter->physics->steerAngle;
    
    frame->speed_ms = adapter->physics->speedKmh / 3.6f;
    frame->engine_rpm = (float)adapter->physics->rpms;
    frame->gear = (int8_t)adapter->physics->gear - 1; // AC gear 0 is Reverse, 1 Neutral, 2 is 1st. In our struct: -1 Rev, 0 N, 1 is 1st.
    
    frame->pos_x = adapter->graphics->carCoordinates[0];
    frame->pos_y = adapter->graphics->carCoordinates[1];
    frame->pos_z = adapter->graphics->carCoordinates[2];
    frame->lap_distance = adapter->graphics->normalizedCarPosition * adapter->graphics->distanceTraveled; // Approximate distance

    for (int i=0; i<4; i++) {
        frame->suspension_travel[i] = adapter->physics->suspensionTravel[i];
        frame->tire_temp_c[i] = adapter->physics->tyreCoreTemperature[i];
        frame->tire_pressure_kpa[i] = adapter->physics->wheelsPressure[i] * 100.0f; // Convert bar/psi to kPa if needed, assuming bar here -> 100 kPa
    }

    return true;
}

void ac_adapter_close(ac_adapter_t* adapter) {
    if (adapter->physics) UnmapViewOfFile(adapter->physics);
    if (adapter->graphics) UnmapViewOfFile(adapter->graphics);
    if (adapter->static_info) UnmapViewOfFile(adapter->static_info);
    
    if (adapter->hPhysics) CloseHandle(adapter->hPhysics);
    if (adapter->hGraphics) CloseHandle(adapter->hGraphics);
    if (adapter->hStatic) CloseHandle(adapter->hStatic);
}

#else
// Stub for non-Windows compilation

bool ac_adapter_init(ac_adapter_t* adapter) { return false; }
bool ac_adapter_read(ac_adapter_t* adapter, rts_telemetry_frame_t* frame) { return false; }
void ac_adapter_close(ac_adapter_t* adapter) {}

#endif
