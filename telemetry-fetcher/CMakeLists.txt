cmake_minimum_required(VERSION 3.10)
project(telemetry_fetcher C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED True)

# Include directories
include_directories(include)

# Source files
set(SOURCES
    src/main.c
    src/storage/rts_writer.c
    src/adapters/ac_adapter.c
    src/adapters/iracing_adapter.c
)

# Executable
add_executable(fetcher ${SOURCES})

if(WIN32)
    target_link_libraries(fetcher kernel32 user32)
endif()
