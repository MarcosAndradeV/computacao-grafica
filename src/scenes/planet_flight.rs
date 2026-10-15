use crate::flight::{
    hud::FlightHUD,
    physics::{FlightControlInput, SpaceshipPhysics},
    terrain::PlanetaryTerrain,
};
use crate::AppState;
use muxapp::muxui::{d3::*, rlgl::*, Matrix, *};
use muxapp::{Input, Scene, SceneAction};

pub struct PlanetFlightScene {
    pub ship: SpaceshipPhysics,
    pub terrain: PlanetaryTerrain,
    pub camera: Camera3D,
    pub is_cockpit_view: bool,
    pub smoothed_cam_pos: Vector3,
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
            terrain: PlanetaryTerrain::default(),
            camera,
            is_cockpit_view: false,
            smoothed_cam_pos: init_cam,
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
        // Modal Scene Actions
        if input.is_action_pressed("pause") {
            return SceneAction::Push("pause".to_string());
        }
        if input.is_action_pressed("switch_scene") {
            return SceneAction::Switch("solar_orbit".to_string());
        }

        // Camera toggle
        if input.is_action_pressed("toggle_camera") {
            self.is_cockpit_view = !self.is_cockpit_view;
        }

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

        let flight_input = FlightControlInput {
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
        };

        // Query planetary terrain surface elevation under the spaceship (water surface or ground)
        let surface_elevation = self.terrain.get_surface_height(self.ship.position.x, self.ship.position.z);

        // Step aerodynamic flight simulation
        self.ship.step(&flight_input, surface_elevation, dt);

        // Update Camera Position & Target using native Vector3 math
        if self.is_cockpit_view {
            // First-Person Cockpit View
            let eye_pos = self.ship.position + self.ship.up * 0.4 + self.ship.forward * 0.3;
            self.camera.position = eye_pos;
            self.camera.target = eye_pos + self.ship.forward * 50.0;
            self.camera.up = self.ship.up;
        } else {
            // Third-Person Dynamic Chase Camera with native .lerp()
            let desired_cam_pos = self.ship.position - self.ship.forward * 16.0 + self.ship.up * 4.5;
            let blend = (dt * 9.0).clamp(0.0, 1.0);
            self.smoothed_cam_pos = self.smoothed_cam_pos.lerp(desired_cam_pos, blend);

            self.camera.position = self.smoothed_cam_pos;
            self.camera.target = self.ship.position + self.ship.forward * 15.0;
            self.camera.up = self.ship.up;
        }

        SceneAction::None
    }

    fn draw_3d(&mut self, _state: &AppState) {
        draw_3d(self.camera, |d3| {
            // 1. Draw Planetary Terrain, Runway, Ocean, and Sky Horizon (batch-safe)
            self.terrain.draw(d3, self.ship.position.x, self.ship.position.z);

            // 2. Draw 3D Spaceship Model in Chase View with safe matrix scoping
            if !self.is_cockpit_view {
                self.draw_spaceship_model(d3);
            }
        });
    }

    fn draw_2d(&mut self, _state: &AppState, style: &Style) {
        FlightHUD::draw(&self.ship, &self.camera, style, self.is_cockpit_view);
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
