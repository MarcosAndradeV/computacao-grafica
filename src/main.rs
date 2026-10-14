use muxapp::{
    App,
    muxui::{rlgl::*, *},
};
use std::f32::consts::PI;

#[derive(Default)]
struct AppState {
    camera: Camera3D,
    rotation_speed: f32,
    earth_rotation: f32,
    earth_orbit_rotation: f32,
    moon_rotation: f32,
    moon_orbit_rotation: f32,

    // Spacecraft dynamics (Linear & Angular velocities)
    velocity: Vector3,
    angular_velocity: Vector3, // x = yaw, y = pitch, z = roll
}

const SUN_RADIUS: f32 = 4.0;
const EARTH_RADIUS: f32 = 0.6;
const EARTH_ORBIT_RADIUS: f32 = 8.0;
const MOON_RADIUS: f32 = 0.16;
const MOON_ORBIT_RADIUS: f32 = 1.5;

// Flight Tuning Parameters
const THRUST_FORWARD: f32 = 40.0;
const THRUST_STRAFE: f32 = 25.0;
const THRUST_VERTICAL: f32 = 25.0;
const LINEAR_DAMPING: f32 = 0.985;  // Slight space drag (0.999 for pure vacuum)
const ANGULAR_DAMPING: f32 = 0.90; // Inertial dampening for turning

const MOUSE_SENSITIVITY: f32 = 0.008;
const ROLL_ACCEL: f32 = 60.0;

fn main() {
    App::init(800, 600, "Solar Orbit - Space Flight", AppState::default)
        .set_virtual_resolution(800, 600)
        .on_load(|s, _| {
            unsafe { MaximizeWindow() };
            s.camera = Camera3D {
                position: Vector3 {
                    x: 0.0,
                    y: 8.0,
                    z: 18.0,
                },
                target: Vector3::default(),
                up: Vector3 {
                    y: 1.0,
                    ..Default::default()
                },
                fovy: 75.0,
                projection: CAMERA_PERSPECTIVE as _,
            };
            s.rotation_speed = 0.8;

            unsafe {
                // Lock and hide cursor for seamless cockpit / mouse look
                DisableCursor();
            }
        })
        .set_fps(60) // 60 FPS provides smoother inertia integration
        .set_style_file("style.gss")
        .on_update(update)
        .run();
}

fn update(s: &mut AppState, style: &Style) {
    let dt = unsafe { GetFrameTime() };

    // Celestial Bodies updates (scaled by dt for framerate independence)
    s.earth_rotation += 60.0 * s.rotation_speed * dt;
    s.earth_orbit_rotation += (365.0 / 360.0) * 20.0 * s.rotation_speed * dt;
    s.moon_rotation += 30.0 * s.rotation_speed * dt;
    s.moon_orbit_rotation += 45.0 * s.rotation_speed * dt;

    unsafe {
        // --- 1. Spacecraft Rotation Input (Pitch, Yaw, Roll) ---
        let mouse_delta = GetMouseDelta();

        // Mouse X -> Yaw, Mouse Y -> Pitch (inverted Y feels natural in flight)
        s.angular_velocity.x += mouse_delta.x * MOUSE_SENSITIVITY;
        s.angular_velocity.y += mouse_delta.y * MOUSE_SENSITIVITY;

        // Roll thrusters: Q / E
        if is_key_down(KEY_Q) {
            s.angular_velocity.z -= ROLL_ACCEL * dt;
        }
        if is_key_down(KEY_E) {
            s.angular_velocity.z += ROLL_ACCEL * dt;
        }

        // Apply angular damping (stabilization gyros)
        s.angular_velocity.x *= ANGULAR_DAMPING;
        s.angular_velocity.y *= ANGULAR_DAMPING;
        s.angular_velocity.z *= ANGULAR_DAMPING;

        // --- 2. Spacecraft Linear Thrust Input ---
        let mut thrust = Vector3::default();

        // Forward / Backward (Surge)
        if is_key_down(KEY_W) {
            thrust.x += THRUST_FORWARD;
        }
        if is_key_down(KEY_S) {
            thrust.x -= THRUST_FORWARD;
        }

        // Strafe Left / Right (Sway)
        if is_key_down(KEY_A) {
            thrust.y -= THRUST_STRAFE;
        }
        if is_key_down(KEY_D) {
            thrust.y += THRUST_STRAFE;
        }

        // Vertical Heave: SPACE = Up, LEFT_SHIFT / C = Down
        if is_key_down(KEY_SPACE) {
            thrust.z += THRUST_VERTICAL;
        }
        if is_key_down(KEY_LEFT_SHIFT) || is_key_down(KEY_C) {
            thrust.z -= THRUST_VERTICAL;
        }

        // Emergency Flight Brake / Kill Drift (X key)
        if is_key_down(KEY_X) {
            s.velocity.x *= 0.85;
            s.velocity.y *= 0.85;
            s.velocity.z *= 0.85;
        }

        // Integrate linear acceleration into velocity
        s.velocity.x += thrust.x * dt;
        s.velocity.y += thrust.y * dt;
        s.velocity.z += thrust.z * dt;

        // Apply space drift damping
        s.velocity.x *= LINEAR_DAMPING.powf(dt * 60.0);
        s.velocity.y *= LINEAR_DAMPING.powf(dt * 60.0);
        s.velocity.z *= LINEAR_DAMPING.powf(dt * 60.0);

        // --- 3. Raylib Pro Camera Update ---
        // Raylib UpdateCameraPro expects movement relative to ship orientation:
        // Movement: x = Forward/Back, y = Right/Left, z = Up/Down
        // Rotation: x = Yaw, y = Pitch, z = Roll
        let movement_step = Vector3 {
            x: s.velocity.x * dt,
            y: s.velocity.y * dt,
            z: s.velocity.z * dt,
        };

        let rotation_step = Vector3 {
            x: s.angular_velocity.x,
            y: s.angular_velocity.y,
            z: s.angular_velocity.z,
        };

        UpdateCameraPro(&mut s.camera, movement_step, rotation_step, 0.0);

        // --- 4. Render Scene ---
        BeginMode3D(s.camera);

        // Sun
        rlPushMatrix();
        rlRotatef(s.earth_rotation * 0.1, 0.0, 1.0, 0.0);
        rlScalef(SUN_RADIUS, SUN_RADIUS, SUN_RADIUS);
        draw_sphere_basic(GOLD);
        rlPopMatrix();

        // Earth Orbit & Earth
        rlPushMatrix();
        rlRotatef(s.earth_orbit_rotation, 0.0, 1.0, 0.0);
        rlTranslatef(EARTH_ORBIT_RADIUS, 0.0, 0.0);

        rlPushMatrix();
        rlRotatef(s.earth_rotation, 0.25, 1.0, 0.0);
        rlScalef(EARTH_RADIUS, EARTH_RADIUS, EARTH_RADIUS);
        draw_sphere_basic(BLUE);
        rlPopMatrix();

        // Moon Orbit & Moon
        rlRotatef(s.moon_orbit_rotation, 0.0, 1.0, 0.0);
        rlTranslatef(MOON_ORBIT_RADIUS, 0.0, 0.0);
        rlRotatef(s.moon_rotation, 0.0, 1.0, 0.0);
        rlScalef(MOON_RADIUS, MOON_RADIUS, MOON_RADIUS);
        draw_sphere_basic(LIGHTGRAY);

        rlPopMatrix();

        DrawGrid(40, 2.0);

        EndMode3D();

        // Reticle HUD
        ReticleHUD.place(style, "hud");

        let speed = (s.velocity.x.powi(2) + s.velocity.y.powi(2) + s.velocity.z.powi(2)).sqrt();
        TextElement::new(
            format!("THRUST SPEED: {:.1} m/s", speed),
            // 10,
            // 30,
            // 18,
            // LIME,
        ).place(style, "thrust_speed");

        DrawFPS(10, 10);
    }
}



fn draw_sphere_basic(color: Color) {
    unsafe {
        let rings: i32 = 16;
        let slices: i32 = 16;

        rlCheckRenderBatchLimit(rings * slices * 6);

        rlBegin(RL_TRIANGLES as _);
        rlColor4ub(color.r, color.g, color.b, color.a);

        let rings_f = rings as f32;
        let slices_f = slices as f32;

        for i in 0..rings {
            let i = i as f32;
            let lat0 = -PI / 2.0 + (PI / rings_f) * i;
            let lat1 = -PI / 2.0 + (PI / rings_f) * (i + 1.0);

            let sin_lat0 = lat0.sin();
            let cos_lat0 = lat0.cos();
            let sin_lat1 = lat1.sin();
            let cos_lat1 = lat1.cos();

            for j in 0..slices {
                let j = j as f32;
                let lon0 = (2.0 * PI / slices_f) * j;
                let lon1 = (2.0 * PI / slices_f) * (j + 1.0);

                let sin_lon0 = lon0.sin();
                let cos_lon0 = lon0.cos();
                let sin_lon1 = lon1.sin();
                let cos_lon1 = lon1.cos();

                // Triangle 1
                rlVertex3f(cos_lat0 * sin_lon0, sin_lat0, cos_lat0 * cos_lon0);
                rlVertex3f(cos_lat1 * sin_lon1, sin_lat1, cos_lat1 * cos_lon1);
                rlVertex3f(cos_lat1 * sin_lon0, sin_lat1, cos_lat1 * cos_lon0);

                // Triangle 2
                rlVertex3f(cos_lat0 * sin_lon0, sin_lat0, cos_lat0 * cos_lon0);
                rlVertex3f(cos_lat0 * sin_lon1, sin_lat0, cos_lat0 * cos_lon1);
                rlVertex3f(cos_lat1 * sin_lon1, sin_lat1, cos_lat1 * cos_lon1);
            }
        }
        rlEnd();
    }
}

struct ReticleHUD;
impl ReticleHUD {
    const RADIUS: f32 = 12.0;
    const THICK: f32 = 1.0;
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
