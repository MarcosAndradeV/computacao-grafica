use crate::AppState;
use muxapp::muxui::{d3::*, *};
use muxapp::{Input, Scene, SceneAction};

pub struct SolarOrbitScene {
    camera: Camera3D,
    rotation_speed: f32,
    earth_rotation: f32,
    earth_orbit_rotation: f32,
    moon_rotation: f32,
    moon_orbit_rotation: f32,

    // Real world-space momentum vector
    world_velocity: Vector3,
    brakes: bool,
}

impl Default for SolarOrbitScene {
    fn default() -> Self {
        Self {
            camera: Camera3DBuilder::new()
                .position(Vector3::new(0.0, 8.0, 18.0))
                .target(Vector3::zero())
                .up(Vector3::new(0.0, 1.0, 0.0))
                .fovy(75.0)
                .build(),
            rotation_speed: 0.8,
            earth_rotation: 0.0,
            earth_orbit_rotation: 0.0,
            moon_rotation: 0.0,
            moon_orbit_rotation: 0.0,
            world_velocity: Vector3::zero(),
            brakes: false,
        }
    }
}

const SUN_RADIUS: f32 = 4.0;
const EARTH_RADIUS: f32 = 0.6;
const EARTH_ORBIT_RADIUS: f32 = 8.0;
const MOON_RADIUS: f32 = 0.16;
const MOON_ORBIT_RADIUS: f32 = 1.5;

// Space Flight Parameters
const MAIN_THRUST: f32 = 60.0; // Forward / Back thrust (W/S)
const STRAFE_THRUST: f32 = 35.0; // Lateral RCS thrusters (A/D)
const VERTICAL_THRUST: f32 = 35.0; // Vertical RCS thrusters (Space/Shift)
const SPACE_DRAG: f32 = 0.992; // Near-vacuum inertia retention (0.999 for hard vacuum)
const MOUSE_SENSITIVITY: f32 = 0.003;
const ROLL_SPEED: f32 = 90.0; // Degrees per second

impl Scene<AppState> for SolarOrbitScene {
    fn on_enter(&mut self, _state: &mut AppState, style: &Style) {
        disable_cursor();
        // Dynamically tune simulation speeds from GSS
        self.rotation_speed = style.get_val_or(&["solar_system", "rotation_speed"], 0.8);
    }

    fn update(
        &mut self,
        _state: &mut AppState,
        input: &Input,
        _style: &Style,
        dt: f32,
    ) -> SceneAction {
        if input.is_action_pressed("pause") {
            return SceneAction::Push("pause".to_string());
        }
        if input.is_action_pressed("switch_scene") {
            return SceneAction::Switch("planet_flight".to_string());
        }

        // Celestial bodies updates
        self.earth_rotation += 60.0 * self.rotation_speed * dt;
        self.earth_orbit_rotation += (365.0 / 360.0) * 20.0 * self.rotation_speed * dt;
        self.moon_rotation += 30.0 * self.rotation_speed * dt;
        self.moon_orbit_rotation += 45.0 * self.rotation_speed * dt;

        // --- 1. Compute Orientation Basis Vectors using native Vector3 math ---
        let forward = (self.camera.target - self.camera.position).normalize();
        let right = forward.cross(self.camera.up).normalize();
        let ship_up = right.cross(forward).normalize();

        // --- 2. Orient Camera (Mouse Pitch/Yaw & Q/E Roll) ---
        let mouse_delta = input.mouse_delta();
        let yaw = mouse_delta.x * MOUSE_SENSITIVITY * 40.0;
        let pitch = mouse_delta.y * MOUSE_SENSITIVITY * 40.0;

        let roll_axis = input.get_axis("roll");
        let roll = roll_axis * ROLL_SPEED * dt;

        unsafe {
            UpdateCameraPro(
                &mut self.camera,
                Vector3::zero(),
                Vector3::new(yaw, pitch, roll),
                0.0,
            );
        }

        // --- 3. Thruster Acceleration along Current Heading ---
        let mut thrust_world = Vector3::zero();

        // Longitudinal thrust
        let throttle_axis = input.get_axis("throttle");
        thrust_world = thrust_world + forward * (throttle_axis * MAIN_THRUST);

        // Lateral strafe thrust
        let strafe_axis = input.get_axis("roll");
        thrust_world = thrust_world + right * (strafe_axis * STRAFE_THRUST);

        // Vertical heave thrust
        let vtol_axis = input.get_axis("vtol");
        thrust_world = thrust_world + ship_up * (vtol_axis * VERTICAL_THRUST);

        // Emergency Flight Brake
        if input.is_action_pressed("airbrake") {
            self.brakes = !self.brakes;
        }

        if self.brakes {
            self.world_velocity = self.world_velocity * 0.90;
        }

        // Integrate acceleration into world velocity vector: v = v + a * dt
        self.world_velocity = self.world_velocity + thrust_world * dt;

        // Apply vacuum inertia drag
        let drag = SPACE_DRAG.powf(dt * 60.0);
        self.world_velocity = self.world_velocity * drag;

        // --- 4. Translate Camera through World Space ---
        let delta_pos = self.world_velocity * dt;
        self.camera.position = self.camera.position + delta_pos;
        self.camera.target = self.camera.target + delta_pos;

        SceneAction::None
    }

    fn draw_3d(&mut self, _state: &AppState) {
        draw_3d(self.camera, |d3| {
            // Sun
            d3.push_matrix(|ctx| {
                ctx.rotate(self.earth_rotation * 0.1, Vector3::new(0.0, 1.0, 0.0));
                ctx.draw_sphere(Vector3::zero(), SUN_RADIUS, GOLD);
                ctx.draw_sphere_wires(Vector3::zero(), SUN_RADIUS + 0.05, 16, 16, YELLOW);
            });

            // Earth Orbit & Earth
            d3.push_matrix(|ctx| {
                ctx.rotate(self.earth_orbit_rotation, Vector3::new(0.0, 1.0, 0.0));
                ctx.translate(Vector3::new(EARTH_ORBIT_RADIUS, 0.0, 0.0));

                ctx.push_matrix(|ctx| {
                    ctx.rotate(self.earth_rotation, Vector3::new(0.25, 1.0, 0.0));
                    ctx.draw_sphere(Vector3::zero(), EARTH_RADIUS, BLUE);
                    ctx.draw_sphere_wires(Vector3::zero(), EARTH_RADIUS + 0.02, 12, 12, SKYBLUE);
                });

                // Moon Orbit & Moon
                ctx.rotate(self.moon_orbit_rotation, Vector3::new(0.0, 1.0, 0.0));
                ctx.translate(Vector3::new(MOON_ORBIT_RADIUS, 0.0, 0.0));
                ctx.rotate(self.moon_rotation, Vector3::new(0.0, 1.0, 0.0));
                ctx.draw_sphere(Vector3::zero(), MOON_RADIUS, LIGHTGRAY);
            });

            d3.draw_grid(60, 2.0);
        });
    }

    fn draw_2d(&mut self, _state: &AppState, style: &Style) {
        // HUD & Flight telemetry
        ReticleHUD.place(style, "hud");

        let speed = self.world_velocity.length();
        TextElement::new(format!("VELOCITY: {:.1} m/s", speed)).place(style, "thrust_speed");

        TextElement::new("SCENE: SOLAR ORBIT | [TAB] PLANET FLIGHT | [P] PAUSE / HELP")
            .place(style, "scene_info");
    }
}

pub struct ReticleHUD;
impl ReticleHUD {
    pub const RADIUS: f32 = 12.0;
    pub const THICK: f32 = 1.0;
}

impl Element for ReticleHUD {
    fn draw(&self, position: Vector2, _style: &Style, _name: &str) {
        unsafe {
            DrawCircleLinesEx(position, Self::RADIUS, Self::THICK, GREEN);
            DrawPixelV(position, GREEN);
        }
    }

    fn measure(&self, _style: &Style, _name: &str) -> Vector2 {
        Vector2::new(Self::RADIUS, Self::RADIUS)
    }
}
