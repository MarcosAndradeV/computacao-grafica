use std::cell::Cell;
use crate::AppState;
use muxapp::muxui::*;
use muxapp::{Input, Scene, SceneAction};

#[derive(Default)]
pub struct PauseOverlayScene {
    resume_requested: Cell<bool>,
    switch_scene_requested: Cell<Option<String>>,
}

impl Scene<AppState> for PauseOverlayScene {
    fn on_enter(&mut self, _state: &mut AppState, _style: &Style) {
        self.resume_requested.set(false);
        self.switch_scene_requested.set(None);
        enable_cursor();
    }

    fn update(
        &mut self,
        _state: &mut AppState,
        input: &Input,
        _style: &Style,
        _dt: f32,
    ) -> SceneAction {
        if let Some(target) = self.switch_scene_requested.take() {
            disable_cursor();
            return SceneAction::Switch(target);
        }

        if input.is_action_pressed("pause") || self.resume_requested.take() {
            disable_cursor();
            return SceneAction::Pop;
        }

        SceneAction::None
    }

    fn draw_2d(&mut self, _state: &AppState, style: &Style) {
        // Semi-transparent dark overlay on top of frozen 3D scene
        draw_rectangle_rec(get_virtual_screen_rect(), get_color(0x000000CC));

        TextElement::new("SIMULATION PAUSED").place(style, "pause_title");
        TextElement::new("Modal stack overlay rendered via MUX SceneRouter")
            .place(style, "pause_subtitle");

        // Action buttons
        let resume_btn = Button::new("Resume Flight (P)");
        let switch_to_planet = Button::new("Go to Planet Flight");
        let switch_to_solar = Button::new("Go to Solar Orbit");

        StackLayout::new(&[
            ("btn_resume", &resume_btn),
            ("btn_planet", &switch_to_planet),
            ("btn_solar", &switch_to_solar),
        ])
        .place(style, "pause_btn_col");

        if resume_btn.clicked() {
            self.resume_requested.set(true);
        }
        if switch_to_planet.clicked() {
            self.switch_scene_requested.set(Some("planet_flight".to_string()));
        }
        if switch_to_solar.clicked() {
            self.switch_scene_requested.set(Some("solar_orbit".to_string()));
        }

        // Control reference card
        RectangleElement.place(style, "help_card");
        TextElement::new(
            "FLIGHT CONTROLS:\n\
             - Mouse / Arrows: Pitch & Roll (Banking)\n\
             - Q / E: Rudder Yaw\n\
             - W / S: Engine Throttle\n\
             - Shift: Afterburner Boost\n\
             - B / X: Airbrakes\n\
             - G: Landing Gear\n\
             - H: Hover Mode (Altitude hold)\n\
             - Space / Ctrl: VTOL Thrusters\n\
             - C: Cycle Camera (Chase / Cockpit / Free)\n\
             - F: Toggle Free Camera (WASD to Fly, Shift Turbo)\n\
             - Tab: Switch Scene\n\
             - F5: Live Reload style.gss",
        )
        .place(style, "help_text");
    }

    fn on_exit(&mut self, _state: &mut AppState) {
        disable_cursor();
    }
}
