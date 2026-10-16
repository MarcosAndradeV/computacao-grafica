mod flight;
mod scenes;

use muxapp::muxui::*;
use muxapp::{App, MouseAxis};
use scenes::{PauseOverlayScene, PlanetFlightScene, SolarOrbitScene};

#[derive(Default)]
pub struct AppState {
    pub debug_info: bool,
}

fn main() {
    App::init(
        800,
        600,
        "Computacao Grafica - Spaceship Planet Flight & Solar Orbit",
        AppState::default,
    )
    .on_load(|_, _| {
        maximize_window();
        disable_exit_key();
    })
    .set_virtual_resolution(800, 600)
    .set_style_file("style.gss")
    .set_fps(60)
    // Register semantic input actions
    .register_action("pause", [KEY_P, KEY_ESCAPE])
    .register_action("switch_scene", [KEY_TAB])
    .register_action("toggle_camera", [KEY_C])
    .register_action("free_camera", [KEY_F])
    .register_action("airbrake", [KEY_B, KEY_X])
    .register_action("landing_gear", [KEY_G])
    .register_action("hover_mode", [KEY_H])
    .register_action("afterburner", [KEY_LEFT_SHIFT])
    // Register continuous control axes (combining discrete keys and continuous mouse movement)
    .register_axis("pitch", KEY_DOWN, KEY_UP)
    .register_mouse_axis("pitch", MouseAxis::Y, -0.025)
    .register_axis("roll", KEY_RIGHT, KEY_LEFT)
    .register_mouse_axis("roll", MouseAxis::X, 0.035)
    .register_axis("yaw", KEY_E, KEY_Q)
    .register_axis("throttle", KEY_W, KEY_S)
    .register_axis("vtol", KEY_SPACE, KEY_LEFT_CONTROL)
    // Register scenes in MUX 0.2.0 SceneRouter
    .add_scene("planet_flight", PlanetFlightScene::default())
    .add_scene("solar_orbit", SolarOrbitScene::default())
    .add_scene("pause", PauseOverlayScene::default())
    .set_active_scene("planet_flight")
    .run();
}
