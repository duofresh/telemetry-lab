#include "rts/rts_writer.h"
#include "adapters/ac_adapter.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <signal.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#endif

volatile sig_atomic_t keep_running = 1;

void handle_sigint(int sig) {
    printf("\nStopping telemetry fetcher...\n");
    keep_running = 0;
}

int main(int argc, char** argv) {
    printf("Assetto Corsa Telemetry Fetcher Started.\n");
    printf("Press Ctrl+C to stop recording and save the file.\n\n");

    signal(SIGINT, handle_sigint);

    ac_adapter_t adapter;
    printf("Waiting for Assetto Corsa to start (make sure you are actually on the track!)...\n");
    
    while (keep_running) {
        if (ac_adapter_init(&adapter)) {
            break;
        }
#ifdef _WIN32
        Sleep(1000);
#endif
    }
    
    if (!keep_running) {
        return 0;
    }
    printf("Connected to Assetto Corsa!\n");

    rts_writer_t writer;
    
    time_t rawtime;
    struct tm * timeinfo;
    char filepath[128];

    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(filepath, sizeof(filepath), "ac_telemetry_%Y%m%d_%H%M%S.rts", timeinfo);

    const char* metadata = "{\"game\": \"Assetto Corsa\"}";
    
    if (!rts_writer_open(&writer, filepath, GAME_ASSETTO_CORSA, metadata)) {
        fprintf(stderr, "Failed to open file %s for writing\n", filepath);
        ac_adapter_close(&adapter);
        return 1;
    }

    printf("Recording telemetry to %s...\n", filepath);

    rts_telemetry_frame_t frame;
    memset(&frame, 0, sizeof(frame));

    int frames_written = 0;
    
    // Poll loop at ~60Hz
    while (keep_running) {
        if (ac_adapter_read(&adapter, &frame)) {
            rts_writer_append(&writer, &frame);
            frames_written++;
        }
        
#ifdef _WIN32
        Sleep(16);
#else
        // Just a stub for non-windows
        break;
#endif
    }

    rts_writer_close(&writer);
    ac_adapter_close(&adapter);
    
    printf("Successfully wrote %d frames to %s.\n", frames_written, filepath);
    return 0;
}
