use bytemuck::{Pod, Zeroable};
use std::fs::File;
use std::io::{self, Read};
use std::path::Path;

#[repr(u8)]
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum GameId {
    Unknown = 0,
    AssettoCorsa = 1,
    ACC = 2,
    IRacing = 3,
    LMU = 4,
}

impl From<u8> for GameId {
    fn from(value: u8) -> Self {
        match value {
            1 => GameId::AssettoCorsa,
            2 => GameId::ACC,
            3 => GameId::IRacing,
            4 => GameId::LMU,
            _ => GameId::Unknown,
        }
    }
}

#[repr(C, packed)]
#[derive(Debug, Clone, Copy, Pod, Zeroable)]
pub struct RtsHeader {
    pub magic: [u8; 4], // "RTS\0"
    pub version: u8,
    pub game_id: u8,
    pub metadata_length: u16,
}

#[repr(C, packed)]
#[derive(Debug, Clone, Copy, Pod, Zeroable)]
pub struct RtsTelemetryFrame {
    pub session_time_ms: u32,
    pub throttle: f32,
    pub brake: f32,
    pub clutch: f32,
    pub steering: f32,
    pub speed_ms: f32,
    pub engine_rpm: f32,
    pub gear: i8,
    pub pos_x: f32,
    pub pos_y: f32,
    pub pos_z: f32,
    pub lap_distance: f32,
    pub suspension_travel: [f32; 4],
    pub tire_temp_c: [f32; 4],
    pub tire_pressure_kpa: [f32; 4],
}

#[derive(Debug)]
pub struct TelemetrySession {
    pub version: u8,
    pub game_id: GameId,
    pub metadata: String,
    pub frames: Vec<RtsTelemetryFrame>,
}

pub fn parse_rts_file<P: AsRef<Path>>(path: P) -> io::Result<TelemetrySession> {
    let mut file = File::open(path)?;

    // Read Header
    let mut header_bytes = [0u8; std::mem::size_of::<RtsHeader>()];
    file.read_exact(&mut header_bytes)?;
    let header: RtsHeader = bytemuck::cast(header_bytes);

    if &header.magic != b"RTS\0" {
        return Err(io::Error::new(
            io::ErrorKind::InvalidData,
            "Invalid RTS magic bytes",
        ));
    }

    // Read Metadata
    let mut metadata_bytes = vec![0u8; header.metadata_length as usize];
    file.read_exact(&mut metadata_bytes)?;
    let metadata = String::from_utf8_lossy(&metadata_bytes).to_string();

    // Read Frames
    let mut frames = Vec::new();
    let frame_size = std::mem::size_of::<RtsTelemetryFrame>();
    let mut frame_bytes = vec![0u8; frame_size];

    while match file.read_exact(&mut frame_bytes) {
        Ok(_) => true,
        Err(e) if e.kind() == io::ErrorKind::UnexpectedEof => false,
        Err(e) => return Err(e),
    } {
        let frame: RtsTelemetryFrame = bytemuck::pod_read_unaligned(&frame_bytes);
        frames.push(frame);
    }

    Ok(TelemetrySession {
        version: header.version,
        game_id: GameId::from(header.game_id),
        metadata,
        frames,
    })
}
