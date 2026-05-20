# 🏎️ Telemetry Lab

A high-performance, low-latency telemetry capture and analysis suite for sim racing games (Assetto Corsa, Assetto Corsa Competizione, iRacing, Le Mans Ultimate, etc.).

This project is built using a **hybrid C/Rust architecture**:
*   **`telemetry-fetcher` (C11):** A lightweight, low-level capture agent that maps game shared memory regions (Windows MMIO) at high frequencies (up to 360Hz) and writes them to disk in a highly packed, proprietary binary stream format (`.rts`).
*   **`telemetry-analyzer` (Rust):** A robust GUI-driven analysis application built using `egui` and `eframe` to parse `.rts` files in microseconds and plot driver inputs, speeds, GPS track maps, and suspension/tire behaviors for lap optimization.

---

## 📁 Repository Structure

```text
telemetry-lab/
├── telemetry-fetcher/           # C capture utility
│   ├── include/
│   │   ├── adapters/            # Windows MMIO mappings for AC & iRacing
│   │   └── rts/                 # Custom file writer & binary structures
│   ├── src/
│   │   ├── adapters/            # Game shared memory readers
│   │   ├── storage/             # File storage logic (.rts format)
│   │   └── main.c               # Capture lifecycle and mock generation
│   ├── CMakeLists.txt
│   └── mingw-toolchain.cmake   # Cross-compilation setup for Windows target
│
├── telemetry-analyzer/          # Rust GUI analysis application
│   ├── src/
│   │   ├── parser.rs            # Zero-allocation .rts file parser
│   │   └── main.rs              # egui desktop GUI & plotting pipeline
│   └── Cargo.toml
│
└── README.md                    # Main documentation
```

---

## 📑 RTS (Racing Telemetry Stream) Binary Specification

The `.rts` file extension represents a high-speed, zero-boilerplate telemetry stream. To achieve maximum throughput and minimal write latency, it writes packed binary representation directly from the CPU register states to disk, conforming to little-endian alignment.

Both the C writer (`rts_writer.c`) and Rust parser (`parser.rs`) align structures strictly to **1-byte boundary alignment**:
*   **C:** `#pragma pack(push, 1)` / `#pragma pack(pop)`
*   **Rust:** `#[repr(C, packed)]` with `bytemuck` safe casts

### Byte-Level File Layout

```text
+------------------------------------------------------------+
|                       RTS HEADER                           |
|  - Magic Signature (4 bytes) "RTS\0"                       |
|  - Version (1 byte)                                        |
|  - Game ID (1 byte)                                        |
|  - Metadata Length N (2 bytes, uint16_t)                   |
+------------------------------------------------------------+
|                     METADATA PAYLOAD                       |
|  - UTF-8 JSON String (N bytes)                             |
+------------------------------------------------------------+
|                 TELEMETRY FRAMES STREAM                    |
|  - Frame 1 (77 bytes, packed telemetry struct)             |
|  - Frame 2 (77 bytes, packed telemetry struct)             |
|  - ...                                                     |
|  - Frame M (77 bytes, packed telemetry struct)             |
+------------------------------------------------------------+
```

---

### 1. RTS File Header Layout (Variable Length)

The header starts with an 8-byte fixed block, followed immediately by a variable-length metadata string payload.

| Offset (Bytes) | Type | Field Name | Description |
| :---: | :---: | :--- | :--- |
| `0x00 - 0x03` | `char[4]` | `magic` | Magic signature bytes: **`'R'`, `'T'`, `'S'`, `'\0'`** |
| `0x04` | `uint8_t` | `version` | File structure version (currently `1`) |
| `0x05` | `uint8_t` | `game_id` | Game enum code (`0`=Unknown, `1`=Assetto Corsa, `2`=ACC, `3`=iRacing, `4`=LMU) |
| `0x06 - 0x07` | `uint16_t` | `metadata_length` | Length `N` of the following metadata payload string (Max 65,535 bytes) |
| `0x08 - (8+N-1)`| `char[N]` | `metadata` | UTF-8 encoded JSON metadata string (e.g., track name, car, driver) |

---

### 2. Standardized Telemetry Frame Layout (77 Bytes Fixed)

Every telemetry sample is recorded as a single contiguous, packed block of **exactly 77 bytes**. To avoid alignment paddings, 1-byte packing is strictly enforced.

| Offset (Bytes) | Field Offset | Type | Field Name | Units / Scale | Description |
| :---: | :---: | :---: | :--- | :---: | :--- |
| **`0`** | `0x00` | `uint32_t` | `session_time_ms` | `ms` | Time elapsed since session start |
| **`4`** | `0x04` | `float` | `throttle` | `[0.0, 1.0]` | Driver throttle position (0% to 100%) |
| **`8`** | `0x08` | `float` | `brake` | `[0.0, 1.0]` | Driver brake pressure/position (0% to 100%) |
| **`12`** | `0x0C` | `float` | `clutch` | `[0.0, 1.0]` | Driver clutch position (0% to 100%) |
| **`16`** | `0x10` | `float` | `steering` | `[-1.0, 1.0]` | Steering angle: negative=left, positive=right |
| **`20`** | `0x14` | `float` | `speed_ms` | `m/s` | Absolute vehicle speed in meters per second |
| **`24`** | `0x18` | `float` | `engine_rpm` | `rpm` | Current engine RPM |
| **`28`** | `0x1C` | `int8_t` | `gear` | `integer` | `-1`=Reverse, `0`=Neutral, `1`+=Forward Gears |
| **`29`** | `0x1D` | `float` | `pos_x` | `meters` | 3D World X coordinates (GPS translation) |
| **`33`** | `0x21` | `float` | `pos_y` | `meters` | 3D World Y coordinates (Altitude) |
| **`37`** | `0x25` | `float` | `pos_z` | `meters` | 3D World Z coordinates (GPS translation) |
| **`41`** | `0x29` | `float` | `lap_distance` | `meters` | Distance traveled from the starting line |
| **`45`** | `0x2D` | `float[4]`| `suspension_travel`| `meters` | Susp. travel array: `[FL, FR, RL, RR]` |
| **`61`** | `0x3D` | `float[4]`| `tire_temp_c` | `°C` | Core tire temperatures: `[FL, FR, RL, RR]` |
| **`77`** | `0x4D` | `float[4]`| `tire_pressure_kpa`| `kPa` | Tires inflation pressures: `[FL, FR, RL, RR]` |

---

## 🛠️ Build and Compilation Instructions

### 1. Compiling the C Telemetry Fetcher
Since sim racing games are predominantly Windows-only, the `telemetry-fetcher` uses Windows shared memory APIs. However, it can be built/cross-compiled on Linux using the MinGW compiler toolchain.

**To Cross-Compile for Windows on Linux:**
Ensure you have `mingw-w64` packages installed on your system.
```bash
cd telemetry-fetcher
mkdir build && cd build
cmake -DCMAKE_TOOLCHAIN_FILE=../mingw-toolchain.cmake ..
make
```
This produces `fetcher.exe` in the build directory, ready to run on any sim racing rig.

---

### 2. Compiling and Running the Rust Analyzer
The analyzer can run natively on both Linux and Windows systems.

**Prerequisites:**
You will need Rust installed on your computer.

```bash
cd telemetry-analyzer
cargo run --release
```
*   Click **"Open File..."** in the GUI to load any `.rts` binary file (e.g. the standard `dummy_telemetry.rts` generated by the fetcher).
*   Interact with the speed graphs, throttle/brake maps, tires thermal state plots, and top-down visual GPS track maps.
