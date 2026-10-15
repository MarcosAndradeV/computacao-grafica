use crate::AppState;
use crate::flight::{
    hud::FlightHUD,
    physics::{FlightControlInput, SpaceshipPhysics},
    terrain::PlanetaryTerrain,
};
use muxapp::muxui::{Matrix, d3::*, rlgl::*, *};
use muxapp::{Input, Scene, SceneAction};

#[derive(Debug, Clone, Copy, PartialEq, Eq, Default)]
pub enum CameraMode {
    #[default]
    Chase,
    Cockpit,
    Free,
}

pub struct PlanetFlightScene {
    pub ship: SpaceshipPhysics,
    pub terrain: PlanetaryTerrain,
    pub camera: Camera3D,
    pub camera_mode: CameraMode,
    pub smoothed_cam_pos: Vector3,
    pub free_cam_pos: Vector3,
    pub free_cam_yaw: f32,
    pub free_cam_pitch: f32,
}

impl Default for PlanetFlightScene {
    fn default() -> Self {
        let ship = SpaceshipPhysics::default();
        let init_cam = ship.position - ship.forward * 20.0 + Vector3::new(0.0, 6.0, 0.0);

        let camera = Camera3DBuilder::new()
            .position(init_cam)
            .target(Vector3::new(0.0, 120.0, 50.0))
            .up(Vector3::new(0.0, 1.0, 0.0))
            .fovy(75.0)
            .build();

        Self {
            ship,
            camera,
            camera_mode: CameraMode::Chase,
            terrain: PlanetaryTerrain::default(),
            smoothed_cam_pos: init_cam,
            free_cam_pos: init_cam,
            free_cam_yaw: 0.0,
            free_cam_pitch: 0.0,
        }
    }
}

impl Scene<AppState> for PlanetFlightScene {
    fn on_enter(&mut self, _state: &mut AppState, style: &Style) {
        disable_cursor();

        // Live tune flight physics from GSS via FromGssValue / GssExt
        self.ship.max_thrust = style.get_val_or(&["flight_physics", "max_thrust"], 38.0);
        self.ship.stall_speed = style.get_val_or(&["flight_physics", "stall_speed"], 28.0);
        self.ship.wing_area = style.get_val_or(&["flight_physics", "wing_area"], 18.0);
        self.ship.boost_multiplier = style.get_val_or(&["flight_physics", "boost_multiplier"], 1.6);
    }

    fn update(
        &mut self,
        _state: &mut AppState,
        input: &Input,
        _style: &Style,
        dt: f32,
    ) -> SceneAction {
        draw_fps(10, 60);
        // Modal Scene Actions
        if input.is_action_pressed("pause") {
            return SceneAction::Push("pause".to_string());
        }
        if input.is_action_pressed("switch_scene") {
            return SceneAction::Switch("solar_orbit".to_string());
        }

        // Camera toggles: C cycles Chase -> Cockpit -> Free; F toggles Free Camera
        if input.is_action_pressed("free_camera") {
            if self.camera_mode == CameraMode::Free {
                self.camera_mode = CameraMode::Chase;
            } else {
                self.enter_free_cam();
            }
        } else if input.is_action_pressed("toggle_camera") {
            self.camera_mode = match self.camera_mode {
                CameraMode::Chase => CameraMode::Cockpit,
                CameraMode::Cockpit => {
                    self.enter_free_cam();
                    CameraMode::Free
                }
                CameraMode::Free => CameraMode::Chase,
            };
        }

        let flight_input = if self.camera_mode == CameraMode::Free {
            // Ship glides on current momentum while pilot navigates free camera
            FlightControlInput::default()
        } else {
            // Continuous analog inputs from composite axes (combining keyboard and mouse delta)
            let pitch_input = input.get_axis("pitch");
            let roll_input = input.get_axis("roll");
            let yaw_input = input.get_axis("yaw");

            // Throttle adjustments from axis
            let throttle_delta = input.get_axis("throttle") * 0.45;

            // VTOL thrusters from axis
            let vtol_axis = input.get_axis("vtol");
            let vtol_up = vtol_axis.max(0.0);
            let vtol_down = (-vtol_axis).max(0.0);

            FlightControlInput {
                pitch: pitch_input.clamp(-1.0, 1.0),
                roll: roll_input.clamp(-1.0, 1.0),
                yaw: yaw_input.clamp(-1.0, 1.0),
                throttle_delta,
                afterburner: input.is_action_down("afterburner"),
                airbrake: input.is_action_pressed("airbrake"),
                vtol_up,
                vtol_down,
                gear_toggle: input.is_action_pressed("landing_gear"),
                hover_mode_toggle: input.is_action_pressed("hover_mode"),
            }
        };

        // Query planetary terrain surface elevation under the spaceship (water surface or ground)
        let surface_elevation = self
            .terrain
            .get_surface_height(self.ship.position.x, self.ship.position.z);

        // Step aerodynamic flight simulation
        if self.camera_mode != CameraMode::Free {
            self.ship.step(&flight_input, surface_elevation, dt);
        }

        // Update Camera Position & Target based on active CameraMode
        match self.camera_mode {
            CameraMode::Free => {
                // Mouse look rotation
                let mouse_delta = input.mouse_delta();
                self.free_cam_yaw -= mouse_delta.x * 0.003;
                self.free_cam_pitch =
                    (self.free_cam_pitch - mouse_delta.y * 0.003).clamp(-1.55, 1.55);

                let cp = self.free_cam_pitch.cos();
                let sp = self.free_cam_pitch.sin();
                let cy = self.free_cam_yaw.cos();
                let sy = self.free_cam_yaw.sin();

                let cam_forward = Vector3::new(cp * sy, sp, cp * cy).normalize();
                let world_up = Vector3::new(0.0, 1.0, 0.0);
                let cam_right = cam_forward.cross(world_up).normalize();
                let cam_up = cam_right.cross(cam_forward).normalize();

                // Directional 6-DOF translation
                let cam_speed = if input.is_action_down("afterburner") {
                    160.0
                } else {
                    50.0
                };
                let mut move_dir = Vector3::zero();

                if is_key_down(KEY_W) || is_key_down(KEY_UP) {
                    move_dir = move_dir + cam_forward;
                }
                if is_key_down(KEY_S) || is_key_down(KEY_DOWN) {
                    move_dir = move_dir - cam_forward;
                }
                if is_key_down(KEY_D) || is_key_down(KEY_RIGHT) {
                    move_dir = move_dir + cam_right;
                }
                if is_key_down(KEY_A) || is_key_down(KEY_LEFT) {
                    move_dir = move_dir - cam_right;
                }
                if is_key_down(KEY_SPACE) || is_key_down(KEY_E) {
                    move_dir = move_dir + world_up;
                }
                if is_key_down(KEY_LEFT_CONTROL) || is_key_down(KEY_Q) {
                    move_dir = move_dir - world_up;
                }

                if move_dir.length_sq() > 0.001 {
                    self.free_cam_pos = self.free_cam_pos + move_dir.normalize() * (cam_speed * dt);
                }

                self.camera.position = self.free_cam_pos;
                self.camera.target = self.free_cam_pos + cam_forward * 10.0;
                self.camera.up = cam_up;
            }
            CameraMode::Cockpit => {
                // First-Person Cockpit View
                let eye_pos = self.ship.position + self.ship.up * 0.4 + self.ship.forward * 0.3;
                self.camera.position = eye_pos;
                self.camera.target = eye_pos + self.ship.forward * 50.0;
                self.camera.up = self.ship.up;
            }
            CameraMode::Chase => {
                // Third-Person Dynamic Chase Camera with native .lerp()
                let desired_cam_pos =
                    self.ship.position - self.ship.forward * 16.0 + self.ship.up * 4.5;
                let blend = (dt * 9.0).clamp(0.0, 1.0);
                self.smoothed_cam_pos = self.smoothed_cam_pos.lerp(desired_cam_pos, blend);

                self.camera.position = self.smoothed_cam_pos;
                self.camera.target = self.ship.position + self.ship.forward * 15.0;
                self.camera.up = self.ship.up;
            }
        }

        SceneAction::None
    }

    fn draw_3d(&mut self, _state: &AppState) {
        draw_3d(self.camera, |d3| {
            // 1. Draw Planetary Terrain, Runway, Ocean, and Sky Horizon (batch-safe)
            self.terrain
                .draw(d3, self.ship.position.x, self.ship.position.z);

            // 2. Draw 3D Spaceship Model in Chase & Free View with safe matrix scoping
            if self.camera_mode != CameraMode::Cockpit {
                self.draw_spaceship_model(d3);
            }
        });
    }

    fn draw_2d(&mut self, _state: &AppState, style: &Style) {
        FlightHUD::draw(&self.ship, &self.camera, style, self.camera_mode);
    }
}

impl PlanetFlightScene {
    fn enter_free_cam(&mut self) {
        self.camera_mode = CameraMode::Free;
        self.free_cam_pos = self.camera.position;
        let forward = (self.camera.target - self.camera.position).normalize();
        self.free_cam_yaw = forward.x.atan2(forward.z);
        self.free_cam_pitch = forward.y.asin().clamp(-1.5, 1.5);
    }
}

impl PlanetFlightScene {
    /// Renders a detailed stylized starfighter spaceship.
    fn draw_spaceship_model(&self, d3: &mut Render3DContext) {
        d3.push_matrix(|ctx| {
            ctx.translate(self.ship.position);

            // Construct and apply orientation matrix from basis vectors
            let m = Matrix::from_basis(
                self.ship.right,
                self.ship.up,
                self.ship.forward,
                Vector3::zero(),
            );
            unsafe {
                rlMultMatrixf(&m as *const Matrix as *const f32);
            }

            // --- Starfighter Geometry ---
            unsafe {
                rlBegin(RL_TRIANGLES as _);

                // Fuselage Main Body (Dark Titanium)
                rlColor4ub(45, 52, 60, 255);
                let nose_z = 4.2;
                let tail_z = -3.2;
                let half_w = 0.9;
                let h = 0.6;

                // Nose cone (pointed forward along +Z)
                rlColor4ub(55, 65, 75, 255);
                // Nose top
                rlVertex3f(0.0, 0.1, nose_z);
                rlVertex3f(-half_w * 0.5, h * 0.4, 0.5);
                rlVertex3f(half_w * 0.5, h * 0.4, 0.5);

                // Nose bottom
                rlVertex3f(0.0, 0.1, nose_z);
                rlVertex3f(half_w * 0.5, -h * 0.4, 0.5);
                rlVertex3f(-half_w * 0.5, -h * 0.4, 0.5);

                // Nose left
                rlVertex3f(0.0, 0.1, nose_z);
                rlVertex3f(-half_w * 0.5, -h * 0.4, 0.5);
                rlVertex3f(-half_w * 0.5, h * 0.4, 0.5);

                // Nose right
                rlVertex3f(0.0, 0.1, nose_z);
                rlVertex3f(half_w * 0.5, h * 0.4, 0.5);
                rlVertex3f(half_w * 0.5, -h * 0.4, 0.5);

                // Cockpit Canopy Glass (Glowing Cyan)
                rlColor4ub(30, 190, 240, 220);
                rlVertex3f(0.0, h * 0.75, 1.8);
                rlVertex3f(-0.35, h * 0.4, 0.4);
                rlVertex3f(0.35, h * 0.4, 0.4);

                rlVertex3f(0.0, h * 0.75, 1.8);
                rlVertex3f(0.35, h * 0.4, 0.4);
                rlVertex3f(0.0, h * 0.65, -0.6);

                rlVertex3f(0.0, h * 0.75, 1.8);
                rlVertex3f(0.0, h * 0.65, -0.6);
                rlVertex3f(-0.35, h * 0.4, 0.4);

                // Delta Wings (Left & Right)
                rlColor4ub(60, 70, 85, 255);
                let wing_span = 4.8;
                let wing_tip_z = -2.6;

                // Left Wing
                rlVertex3f(-0.6, 0.0, 1.2);
                rlVertex3f(-wing_span, 0.1, wing_tip_z);
                rlVertex3f(-0.7, 0.0, tail_z);

                rlVertex3f(-0.6, 0.0, 1.2);
                rlVertex3f(-0.7, 0.0, tail_z);
                rlVertex3f(-wing_span, 0.1, wing_tip_z);

                // Right Wing
                rlVertex3f(0.6, 0.0, 1.2);
                rlVertex3f(0.7, 0.0, tail_z);
                rlVertex3f(wing_span, 0.1, wing_tip_z);

                rlVertex3f(0.6, 0.0, 1.2);
                rlVertex3f(wing_span, 0.1, wing_tip_z);
                rlVertex3f(0.7, 0.0, tail_z);

                // Twin Canted Vertical Stabilizers / Fins
                rlColor4ub(75, 88, 105, 255);
                let fin_h = 1.4;

                // Left Fin
                rlVertex3f(-0.7, 0.1, tail_z + 1.2);
                rlVertex3f(-1.1, fin_h, tail_z);
                rlVertex3f(-0.8, 0.1, tail_z);

                // Right Fin
                rlVertex3f(0.7, 0.1, tail_z + 1.2);
                rlVertex3f(0.8, 0.1, tail_z);
                rlVertex3f(1.1, fin_h, tail_z);

                // Engine Exhaust Nozzles
                rlColor4ub(25, 25, 28, 255);
                rlVertex3f(-0.5, 0.3, tail_z);
                rlVertex3f(-0.2, -0.3, tail_z);
                rlVertex3f(-0.5, -0.3, tail_z);

                rlVertex3f(0.2, 0.3, tail_z);
                rlVertex3f(0.5, -0.3, tail_z);
                rlVertex3f(0.2, -0.3, tail_z);

                // Engine Plasma Flame (Length scales with throttle)
                if self.ship.throttle > 0.05 {
                    let flame_len = 1.0 + self.ship.throttle * 4.2;
                    rlColor4ub(60, 160, 255, 230); // Ion blue core

                    // Left Engine plume
                    rlVertex3f(-0.35, 0.0, tail_z);
                    rlVertex3f(-0.35, 0.0, tail_z - flame_len);
                    rlVertex3f(-0.5, 0.0, tail_z);

                    // Right Engine plume
                    rlVertex3f(0.35, 0.0, tail_z);
                    rlVertex3f(0.5, 0.0, tail_z);
                    rlVertex3f(0.35, 0.0, tail_z - flame_len);
                }

                rlEnd();
            }
        });
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_camera_mode_default_is_chase() {
        assert_eq!(CameraMode::default(), CameraMode::Chase);
    }

    #[test]
    fn test_camera_mode_cycle() {
        let mut mode = CameraMode::Chase;
        mode = match mode {
            CameraMode::Chase => CameraMode::Cockpit,
            CameraMode::Cockpit => CameraMode::Free,
            CameraMode::Free => CameraMode::Chase,
        };
        assert_eq!(mode, CameraMode::Cockpit);

        mode = match mode {
            CameraMode::Chase => CameraMode::Cockpit,
            CameraMode::Cockpit => CameraMode::Free,
            CameraMode::Free => CameraMode::Chase,
        };
        assert_eq!(mode, CameraMode::Free);

        mode = match mode {
            CameraMode::Chase => CameraMode::Cockpit,
            CameraMode::Cockpit => CameraMode::Free,
            CameraMode::Free => CameraMode::Chase,
        };
        assert_eq!(mode, CameraMode::Chase);
    }

    #[test]
    fn test_enter_free_cam() {
        let mut scene = PlanetFlightScene::default();
        assert_eq!(scene.camera_mode, CameraMode::Chase);

        scene.enter_free_cam();
        assert_eq!(scene.camera_mode, CameraMode::Free);
        assert_eq!(scene.free_cam_pos, scene.camera.position);
    }
}
