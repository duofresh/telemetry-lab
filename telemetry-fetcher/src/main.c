#include "rts/rts_writer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

void generate_dummy_data(const char* filepath) {
    rts_writer_t writer;
    const char* metadata = "{\"track\": \"Spa-Francorchamps\", \"car\": \"Porsche 911 GT3 R\"}";
    
    if (!rts_writer_open(&writer, filepath, GAME_ACC, metadata)) {
        fprintf(stderr, "Failed to open file %s for writing\n", filepath);
        return;
    }

    printf("Generating dummy telemetry data to %s...\n", filepath);

    rts_telemetry_frame_t frame;
    memset(&frame, 0, sizeof(frame));

    // Generate 60 seconds of dummy data at 60Hz
    int num_frames = 60 * 60;
    
    for (int i = 0; i < num_frames; ++i) {
        frame.session_time_ms = i * (1000 / 60);
        
        // Sine wave for speed
        float time_s = frame.session_time_ms / 1000.0f;
        frame.speed_ms = 50.0f + 20.0f * sinf(time_s * 0.5f); // Oscillate between 30 and 70 m/s
        
        frame.engine_rpm = 6000.0f + 2000.0f * sinf(time_s * 2.0f);
        frame.gear = 4;
        
        frame.throttle = (sinf(time_s) > 0) ? 1.0f : 0.0f;
        frame.brake = (sinf(time_s) < 0) ? 1.0f : 0.0f;
        frame.steering = 0.5f * sinf(time_s * 0.2f);
        
        frame.lap_distance = time_s * 50.0f; // Approx distance
        
        rts_writer_append(&writer, &frame);
    }

    rts_writer_close(&writer);
    printf("Successfully wrote %d frames.\n", num_frames);
}

int main(int argc, char** argv) {
    printf("Telemetry Fetcher Started.\n");
    generate_dummy_data("dummy_telemetry.rts");
    return 0;
}
