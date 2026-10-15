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

    // Real world-space momentum vector
    world_velocity: Vector3,

    brakes: bool,
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

fn main() {
    App::init(
        800,
        600,
        "Solar Orbit - Real Space Flight",
        AppState::default,
    )
    .set_virtual_resolution(800, 600)
    .on_load(|s, _| {
        unsafe { MaximizeWindow() };
        s.camera = Camera3D {
            position: Vector3 {
                x: 0.0,
                y: 8.0,
                z: 18.0,
            },
            target: Vector3 {
                x: 0.0,
                y: 0.0,
                z: 0.0,
            },
            up: Vector3 {
                y: 1.0,
                ..Default::default()
            },
            fovy: 75.0,
            projection: CAMERA_PERSPECTIVE as _,
        };
        s.rotation_speed = 0.8;

        unsafe {
            DisableCursor();
        }
    })
    .set_fps(60)
    .set_style_file("style.gss")
    .on_update(update)
    .run();
}

fn update(s: &mut AppState, style: &Style) {
    let dt = unsafe { GetFrameTime() };

    // Celestial bodies updates
    s.earth_rotation += 60.0 * s.rotation_speed * dt;
    s.earth_orbit_rotation += (365.0 / 360.0) * 20.0 * s.rotation_speed * dt;
    s.moon_rotation += 30.0 * s.rotation_speed * dt;
    s.moon_orbit_rotation += 45.0 * s.rotation_speed * dt;

    unsafe {
        // --- 1. Compute Orientation Basis Vectors (Forward, Right, Up) ---
        let mut forward = Vector3 {
            x: s.camera.target.x - s.camera.position.x,
            y: s.camera.target.y - s.camera.position.y,
            z: s.camera.target.z - s.camera.position.z,
        };
        let f_len = (forward.x * forward.x + forward.y * forward.y + forward.z * forward.z).sqrt();
        if f_len > 0.0001 {
            forward.x /= f_len;
            forward.y /= f_len;
            forward.z /= f_len;
        }

        // Right = Forward x Up
        let up = s.camera.up;
        let mut right = Vector3 {
            x: forward.y * up.z - forward.z * up.y,
            y: forward.z * up.x - forward.x * up.z,
            z: forward.x * up.y - forward.y * up.x,
        };
        let r_len = (right.x * right.x + right.y * right.y + right.z * right.z).sqrt();
        if r_len > 0.0001 {
            right.x /= r_len;
            right.y /= r_len;
            right.z /= r_len;
        }

        // True Ship Up = Right x Forward
        let ship_up = Vector3 {
            x: right.y * forward.z - right.z * forward.y,
            y: right.z * forward.x - right.x * forward.z,
            z: right.x * forward.y - right.y * forward.x,
        };

        // --- 2. Orient Camera (Mouse Pitch/Yaw & Q/E Roll) ---
        let mouse_delta = GetMouseDelta();
        let yaw = mouse_delta.x * MOUSE_SENSITIVITY * 40.0;
        let pitch = mouse_delta.y * MOUSE_SENSITIVITY * 40.0;

        let mut roll = 0.0;
        if is_key_down(KEY_Q) {
            roll -= ROLL_SPEED * dt;
        }
        if is_key_down(KEY_E) {
            roll += ROLL_SPEED * dt;
        }

        // Update camera orientation with 0 positional movement
        UpdateCameraPro(
            &mut s.camera,
            Vector3::default(),
            Vector3 {
                x: yaw,
                y: pitch,
                z: roll,
            },
            0.0,
        );

        // --- 3. Thruster Acceleration along Current Heading ---
        let mut thrust_world = Vector3::default();

        // W/S: Accelerate strictly in forward/backward direction of ship's nose
        if is_key_down(KEY_W) {
            thrust_world.x += forward.x * MAIN_THRUST;
            thrust_world.y += forward.y * MAIN_THRUST;
            thrust_world.z += forward.z * MAIN_THRUST;
        }
        if is_key_down(KEY_S) {
            thrust_world.x -= forward.x * MAIN_THRUST;
            thrust_world.y -= forward.y * MAIN_THRUST;
            thrust_world.z -= forward.z * MAIN_THRUST;
        }

        // A/D: Strafe left/right along ship lateral axis
        if is_key_down(KEY_D) {
            thrust_world.x += right.x * STRAFE_THRUST;
            thrust_world.y += right.y * STRAFE_THRUST;
            thrust_world.z += right.z * STRAFE_THRUST;
        }
        if is_key_down(KEY_A) {
            thrust_world.x -= right.x * STRAFE_THRUST;
            thrust_world.y -= right.y * STRAFE_THRUST;
            thrust_world.z -= right.z * STRAFE_THRUST;
        }

        // SPACE / SHIFT: Vertical heave along ship roof/floor axis
        if is_key_down(KEY_SPACE) {
            thrust_world.x += ship_up.x * VERTICAL_THRUST;
            thrust_world.y += ship_up.y * VERTICAL_THRUST;
            thrust_world.z += ship_up.z * VERTICAL_THRUST;
        }
        if is_key_down(KEY_LEFT_SHIFT) || is_key_down(KEY_C) {
            thrust_world.x -= ship_up.x * VERTICAL_THRUST;
            thrust_world.y -= ship_up.y * VERTICAL_THRUST;
            thrust_world.z -= ship_up.z * VERTICAL_THRUST;
        }

        // Emergency Flight Brake
        if is_key_pressed(KEY_X) {
            s.brakes = !s.brakes;
        }

        if s.brakes {
            s.world_velocity.x *= 0.90;
            s.world_velocity.y *= 0.90;
            s.world_velocity.z *= 0.90;
        }

        // Integrate acceleration into world velocity vector: v = v + a * dt
        s.world_velocity.x += thrust_world.x * dt;
        s.world_velocity.y += thrust_world.y * dt;
        s.world_velocity.z += thrust_world.z * dt;

        // Apply vacuum inertia drag
        let drag = SPACE_DRAG.powf(dt * 60.0);
        s.world_velocity.x *= drag;
        s.world_velocity.y *= drag;
        s.world_velocity.z *= drag;

        // --- 4. Translate Camera through World Space ---
        let delta_pos = Vector3 {
            x: s.world_velocity.x * dt,
            y: s.world_velocity.y * dt,
            z: s.world_velocity.z * dt,
        };

        s.camera.position.x += delta_pos.x;
        s.camera.position.y += delta_pos.y;
        s.camera.position.z += delta_pos.z;

        s.camera.target.x += delta_pos.x;
        s.camera.target.y += delta_pos.y;
        s.camera.target.z += delta_pos.z;

        // --- 5. Render Scene ---
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

        DrawGrid(60, 2.0);

        EndMode3D();

        // HUD & Flight telemetry
        ReticleHUD.place(style, "hud");

        let speed =
            (s.world_velocity.x.powi(2) + s.world_velocity.y.powi(2) + s.world_velocity.z.powi(2))
                .sqrt();

        TextElement::new(format!("VELOCITY: {:.1} m/s", speed)).place(style, "thrust_speed");

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

                rlVertex3f(cos_lat0 * sin_lon0, sin_lat0, cos_lat0 * cos_lon0);
                rlVertex3f(cos_lat1 * sin_lon1, sin_lat1, cos_lat1 * cos_lon1);
                rlVertex3f(cos_lat1 * sin_lon0, sin_lat1, cos_lat1 * cos_lon0);

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
