#ifndef RTS_TELEMETRY_FRAME_H
#define RTS_TELEMETRY_FRAME_H

#include <stdint.h>

#pragma pack(push, 1)

// Game identifiers
typedef enum {
    GAME_UNKNOWN = 0,
    GAME_ASSETTO_CORSA = 1,
    GAME_ACC = 2,
    GAME_IRACING = 3,
    GAME_LMU = 4
} rts_game_id_t;

// RTS File Header (Variable Length in reality, but this is the fixed part)
typedef struct {
    char magic[4]; // "RTS\0"
    uint8_t version; // e.g., 1
    uint8_t game_id; // rts_game_id_t
    uint16_t metadata_length;
    // Metadata string payload follows immediately after (UTF-8, JSON or key-value)
} rts_header_t;

// Standardized Telemetry Frame
typedef struct {
    uint32_t session_time_ms; // Time since session start

    // Driver Inputs (0.0 to 1.0)
    float throttle;
    float brake;
    float clutch;
    float steering; // -1.0 to 1.0

    // Vehicle State
    float speed_ms; // Speed in m/s
    float engine_rpm;
    int8_t gear;    // -1 = Reverse, 0 = Neutral, 1+ = Forward gears

    // Spatial State
    float pos_x;
    float pos_y;
    float pos_z;
    float lap_distance; // Distance around the track (normalized or absolute meters)

    // Suspension & Tires (FL, FR, RL, RR)
    float suspension_travel[4]; // Meters
    float tire_temp_c[4];       // Celsius
    float tire_pressure_kpa[4]; // kPa

} rts_telemetry_frame_t;

#pragma pack(pop)

#endif // RTS_TELEMETRY_FRAME_H
