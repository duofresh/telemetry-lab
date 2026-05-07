mod parser;

use parser::{parse_rts_file, TelemetrySession};
use eframe::egui;
use egui_plot::{Line, Plot, PlotPoints, Points};

fn main() -> eframe::Result {
    println!("Telemetry Analyzer Starting...");

    // Test reading the dummy file
    let path = "../telemetry-fetcher/dummy_telemetry.rts";
    let session = match parse_rts_file(path) {
        Ok(s) => Some(s),
        Err(e) => {
            eprintln!("Failed to parse RTS file: {}", e);
            None
        }
    };

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

struct AnalyzerApp {
    session: Option<TelemetrySession>,
    selected_lap: usize,
}

impl AnalyzerApp {
    fn new(session: Option<TelemetrySession>) -> Self {
        Self { session, selected_lap: 0 }
    }

    fn plot_speed(&self, ui: &mut egui::Ui) {
        if let Some(session) = &self.session {
            let points: PlotPoints = session.frames.iter()
                .map(|f| [f.lap_distance as f64, f.speed_ms as f64 * 3.6]) // convert to km/h
                .collect();
            let line = Line::new(points).name("Speed (km/h)").color(egui::Color32::LIGHT_BLUE);
            
            Plot::new("speed_plot")
                .view_aspect(4.0)
                .show(ui, |plot_ui| plot_ui.line(line));
        }
    }

    fn plot_inputs(&self, ui: &mut egui::Ui) {
        if let Some(session) = &self.session {
            let throttle_points: PlotPoints = session.frames.iter()
                .map(|f| [f.lap_distance as f64, f.throttle as f64 * 100.0])
                .collect();
            let brake_points: PlotPoints = session.frames.iter()
                .map(|f| [f.lap_distance as f64, f.brake as f64 * 100.0])
                .collect();
            
            let throttle_line = Line::new(throttle_points).name("Throttle %").color(egui::Color32::GREEN);
            let brake_line = Line::new(brake_points).name("Brake %").color(egui::Color32::RED);

            Plot::new("inputs_plot")
                .view_aspect(4.0)
                .show(ui, |plot_ui| {
                    plot_ui.line(throttle_line);
                    plot_ui.line(brake_line);
                });
        }
    }

    fn plot_gps(&self, ui: &mut egui::Ui) {
        if let Some(session) = &self.session {
            let points: PlotPoints = session.frames.iter()
                .map(|f| [f.pos_x as f64, f.pos_z as f64]) // top-down view
                .collect();
            
            let track = Points::new(points).name("Racing Line").color(egui::Color32::YELLOW).radius(2.0);

            Plot::new("gps_plot")
                .data_aspect(1.0)
                .show(ui, |plot_ui| {
                    plot_ui.points(track);
                });
        }
    }

    fn plot_tires(&self, ui: &mut egui::Ui) {
        if let Some(session) = &self.session {
            // Plot FL tire temp
            let fl_temp_points: PlotPoints = session.frames.iter()
                .map(|f| [f.lap_distance as f64, f.tire_temp_c[0] as f64])
                .collect();
            
            let fl_line = Line::new(fl_temp_points).name("FL Temp (°C)").color(egui::Color32::LIGHT_RED);

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
                if let Some(session) = &self.session {
                    ui.label(format!("Game: {:?}", session.game_id));
                    ui.label(format!("Frames: {}", session.frames.len()));
                } else {
                    ui.label("No data loaded.");
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
