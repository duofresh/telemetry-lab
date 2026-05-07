#include "rts/rts_writer.h"
#include <string.h>

bool rts_writer_open(rts_writer_t* writer, const char* filepath, rts_game_id_t game_id, const char* metadata) {
    if (!writer || !filepath) return false;

    writer->file = fopen(filepath, "wb");
    if (!writer->file) return false;

    rts_header_t header;
    header.magic[0] = 'R';
    header.magic[1] = 'T';
    header.magic[2] = 'S';
    header.magic[3] = '\0';
    header.version = 1;
    header.game_id = (uint8_t)game_id;
    
    size_t metadata_len = metadata ? strlen(metadata) : 0;
    if (metadata_len > 65535) metadata_len = 65535; // Cap at uint16_t max
    header.metadata_length = (uint16_t)metadata_len;

    // Write Header
    if (fwrite(&header, sizeof(rts_header_t), 1, writer->file) != 1) {
        fclose(writer->file);
        writer->file = NULL;
        return false;
    }

    // Write Metadata payload if any
    if (metadata_len > 0) {
        if (fwrite(metadata, 1, metadata_len, writer->file) != metadata_len) {
            fclose(writer->file);
            writer->file = NULL;
            return false;
        }
    }

    return true;
}

bool rts_writer_append(rts_writer_t* writer, const rts_telemetry_frame_t* frame) {
    if (!writer || !writer->file || !frame) return false;

    if (fwrite(frame, sizeof(rts_telemetry_frame_t), 1, writer->file) != 1) {
        return false;
    }

    return true;
}

void rts_writer_close(rts_writer_t* writer) {
    if (writer && writer->file) {
        fclose(writer->file);
        writer->file = NULL;
    }
}
