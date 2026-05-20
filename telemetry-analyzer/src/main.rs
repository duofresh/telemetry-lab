mod parser;

use parser::{parse_rts_file, TelemetrySession};
use eframe::egui;
use egui_plot::{Line, Plot, PlotPoints, Points};

fn main() -> eframe::Result<()> {
    println!("Telemetry Analyzer Starting...");

    let session = None; // Start without a file loaded

    let options = eframe::NativeOptions {
        viewport: egui::ViewportBuilder::default().with_inner_size([1200.0, 800.0]),
        ..Default::default()
    };

    eframe::run_native(
        "Telemetry Analyzer",
        options,
        Box::new(|_cc| Box::new(AnalyzerApp::new(session))),
    )
}

struct CachedPlots {
    speed: PlotPoints,
    throttle: PlotPoints,
    brake: PlotPoints,
    gps: PlotPoints,
    fl_temp: PlotPoints,
}

impl CachedPlots {
    fn from_session(session: &TelemetrySession) -> Self {
        let speed = session.frames.iter()
            .map(|f| [f.lap_distance as f64, f.speed_ms as f64 * 3.6]) // convert to km/h
            .collect::<Vec<[f64; 2]>>();
        
        let throttle = session.frames.iter()
            .map(|f| [f.lap_distance as f64, f.throttle as f64 * 100.0])
            .collect::<Vec<[f64; 2]>>();
        
        let brake = session.frames.iter()
            .map(|f| [f.lap_distance as f64, f.brake as f64 * 100.0])
            .collect::<Vec<[f64; 2]>>();
        
        let gps = session.frames.iter()
            .map(|f| [f.pos_x as f64, f.pos_z as f64]) // top-down view
            .collect::<Vec<[f64; 2]>>();
        
        let fl_temp = session.frames.iter()
            .map(|f| [f.lap_distance as f64, f.tire_temp_c[0] as f64])
            .collect::<Vec<[f64; 2]>>();
            
        Self {
            speed: PlotPoints::new(speed),
            throttle: PlotPoints::new(throttle),
            brake: PlotPoints::new(brake),
            gps: PlotPoints::new(gps),
            fl_temp: PlotPoints::new(fl_temp),
        }
    }
}

struct AnalyzerApp {
    session: Option<TelemetrySession>,
    selected_lap: usize,
    cached_plots: Option<CachedPlots>,
}

impl AnalyzerApp {
    fn new(session: Option<TelemetrySession>) -> Self {
        let cached_plots = session.as_ref().map(CachedPlots::from_session);
        Self {
            session,
            selected_lap: 0,
            cached_plots,
        }
    }

    fn plot_speed(&self, ui: &mut egui::Ui) {
        if let Some(cached) = &self.cached_plots {
            let line = Line::new(cached.speed.clone()).name("Speed (km/h)").color(egui::Color32::LIGHT_BLUE);
            
            Plot::new("speed_plot")
                .view_aspect(4.0)
                .show(ui, |plot_ui| plot_ui.line(line));
        }
    }

    fn plot_inputs(&self, ui: &mut egui::Ui) {
        if let Some(cached) = &self.cached_plots {
            let throttle_line = Line::new(cached.throttle.clone()).name("Throttle %").color(egui::Color32::GREEN);
            let brake_line = Line::new(cached.brake.clone()).name("Brake %").color(egui::Color32::RED);

            Plot::new("inputs_plot")
                .view_aspect(4.0)
                .show(ui, |plot_ui| {
                    plot_ui.line(throttle_line);
                    plot_ui.line(brake_line);
                });
        }
    }

    fn plot_gps(&self, ui: &mut egui::Ui) {
        if let Some(cached) = &self.cached_plots {
            let track = Points::new(cached.gps.clone()).name("Racing Line").color(egui::Color32::YELLOW).radius(2.0);

            Plot::new("gps_plot")
                .data_aspect(1.0)
                .show(ui, |plot_ui| {
                    plot_ui.points(track);
                });
        }
    }

    fn plot_tires(&self, ui: &mut egui::Ui) {
        if let Some(cached) = &self.cached_plots {
            let fl_line = Line::new(cached.fl_temp.clone()).name("FL Temp (°C)").color(egui::Color32::LIGHT_RED);

            Plot::new("tire_plot")
                .view_aspect(4.0)
                .show(ui, |plot_ui| {
                    plot_ui.line(fl_line);
                });
        }
    }
}

impl eframe::App for AnalyzerApp {
    fn update(&mut self, ctx: &egui::Context, _frame: &mut eframe::Frame) {
        egui::TopBottomPanel::top("top_panel").show(ctx, |ui| {
            ui.horizontal(|ui| {
                ui.heading("Telemetry Analyzer");
                
                if ui.button("Open File...").clicked() {
                    if let Some(path) = rfd::FileDialog::new()
                        .add_filter("RTS Telemetry", &["rts"])
                        .set_directory("../telemetry-fetcher")
                        .pick_file() 
                    {
                        match parse_rts_file(&path) {
                            Ok(s) => {
                                self.cached_plots = Some(CachedPlots::from_session(&s));
                                self.session = Some(s);
                            }
                            Err(e) => eprintln!("Failed to open file: {}", e),
                        }
                    }
                }

                ui.separator();

                if let Some(session) = &self.session {
                    ui.label(format!("Game: {:?}", session.game_id));
                    ui.label(format!("Frames: {}", session.frames.len()));
                } else {
                    ui.label("No data loaded. Please open a file.");
                }
            });
        });

        egui::SidePanel::left("side_panel").exact_width(300.0).show(ctx, |ui| {
            ui.heading("Track Map (GPS)");
            ui.add_space(10.0);
            self.plot_gps(ui);
            
            ui.add_space(20.0);
            ui.heading("Insights");
            ui.label("Min Corner Speed: 85 km/h (Turn 1)");
            ui.label("Tire Wear: Good (FL 98%)");
        });

        egui::CentralPanel::default().show(ctx, |ui| {
            egui::ScrollArea::vertical().show(ui, |ui| {
                ui.heading("Speed & Gear");
                self.plot_speed(ui);
                
                ui.add_space(20.0);
                ui.heading("Driver Inputs");
                self.plot_inputs(ui);

                ui.add_space(20.0);
                ui.heading("Tire Temperatures");
                self.plot_tires(ui);
            });
        });
    }
}
