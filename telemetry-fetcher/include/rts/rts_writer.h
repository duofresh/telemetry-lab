#ifndef RTS_WRITER_H
#define RTS_WRITER_H

#include "rts/telemetry_frame.h"
#include <stdio.h>
#include <stdbool.h>

typedef struct {
    FILE* file;
} rts_writer_t;

// Open a new RTS file for writing, writes the header
bool rts_writer_open(rts_writer_t* writer, const char* filepath, rts_game_id_t game_id, const char* metadata);

// Append a frame to the file
bool rts_writer_append(rts_writer_t* writer, const rts_telemetry_frame_t* frame);

// Close the file
void rts_writer_close(rts_writer_t* writer);

#endif // RTS_WRITER_H
