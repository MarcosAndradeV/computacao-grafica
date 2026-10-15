use crate::flight::physics::SpaceshipPhysics;
use crate::scenes::CameraMode;
use muxapp::muxui::*;
use std::f32::consts::PI;

pub struct FlightHUD;

impl FlightHUD {
    /// Renders the complete flight instrument suite on top of the 3D scene.
    pub fn draw(ship: &SpaceshipPhysics, camera: &Camera3D, style: &Style, camera_mode: CameraMode) {
        let sw = get_virtual_screen_width();
        let sh = get_virtual_screen_height();
        let cx = sw * 0.5;
        let cy = sh * 0.5;

        // 1. Center Reticle & Artificial Horizon
        Self::draw_artificial_horizon(ship, cx, cy);

        // 2. Velocity Vector / Flight Path Marker
        Self::draw_flight_path_marker(ship, cx, cy);

        // 3. Top Compass Ribbon Box
        Self::draw_compass_frame(cx, 32.0);

        // 4. Airspeed & Altitude Tape Frames
        Self::draw_tape_frames(cx, cy, sw);

        // --- Mux UI Declarative Elements (Safely null-terminated & GSS positioned) ---

        // Top Compass heading label
        let mut heading = ship.forward.x.atan2(ship.forward.z) * 180.0 / PI;
        if heading < 0.0 {
            heading += 360.0;
        }
        TextElement::new(format!("HDG: {:03.0}°", heading)).place(style, "compass_heading");

        // Free Camera active guidance hint
        if camera_mode == CameraMode::Free {
            TextElement::new("FREE CAM: Mouse Look | WASD: Move | Space/Ctrl: Elevation | Shift: Turbo | F/C: Exit")
                .place(style, "free_cam_hint");
        }

        // Airspeed Tape Elements using MUX 0.3.0 dynamic with_class
        TextElement::new("SPD").place(style, "tape_speed_title");
        TextElement::new(format!("{:.0}", ship.airspeed))
            .with_class(if ship.is_stalling { "alert" } else { "normal" })
            .place(style, "tape_speed_val");
        if ship.is_stalling {
            TextElement::new("STALL").place(style, "tape_stall_alert");
        }

        // Altitude Tape Elements
        TextElement::new("ALT").place(style, "tape_alt_title");
        TextElement::new(format!("{:.0}", ship.altitude_msl)).place(style, "tape_alt_val");
        TextElement::new(format!("R: {:.0}", ship.altitude_agl)).place(style, "tape_alt_agl");
        TextElement::new(format!("VS: {:+.1}", ship.vertical_speed)).place(style, "tape_alt_vs");

        // Scene and Camera info banner (Top Left)
        let view_str = match camera_mode {
            CameraMode::Cockpit => "COCKPIT [C]",
            CameraMode::Chase => "CHASE CAM [C]",
            CameraMode::Free => "FREE CAM [C/F]",
        };
        TextElement::new(format!(
            "SCENE: PLANET FLIGHT | VIEW: {} | [TAB] SWITCH | [P] PAUSE",
            view_str
        ))
        .place(style, "scene_info");

        // Throttle readout (Bottom Left)
        TextElement::new(format!("THROTTLE: {:>3.0}%", ship.throttle * 100.0))
            .place(style, "hud_throttle");

        // Lower Telemetry Status Bar using Mux UI StackLayout & dynamic with_class
        let gear_str = if ship.gear_down { "GEAR: DOWN [G]" } else { "GEAR: UP [G]" };
        let brake_str = if ship.airbrake { "AIRBRAKE: ON [B]" } else { "AIRBRAKE: OFF [B]" };
        let hover_str = if ship.hover_mode { "HOVER: ACTIVE [H]" } else { "HOVER: OFF [H]" };
        let g_str = format!("G-FORCE: {:.1}G", ship.g_force);

        let gear_elem = TextElement::new(gear_str)
            .with_class(if ship.gear_down { "active" } else { "standby" });
        let brake_elem = TextElement::new(brake_str)
            .with_class(if ship.airbrake { "alert" } else { "standby" });
        let hover_elem = TextElement::new(hover_str)
            .with_class(if ship.hover_mode { "cyan" } else { "standby" });
        let g_elem = TextElement::new(g_str)
            .with_class("gold");

        StackLayout::new(&[
            ("hud_badge", &gear_elem),
            ("hud_badge", &brake_elem),
            ("hud_badge", &hover_elem),
            ("hud_badge", &g_elem),
        ])
        .place(style, "telemetry_bar");

        // 3D In-World Runway Waypoint HUD Projection (MUX 0.3.0 get_virtual_world_to_screen)
        let runway_pos = Vector3::new(0.0, 1.0, 0.0);
        if let Some(screen_pos) = get_virtual_world_to_screen(camera, runway_pos) {
            let dist_km = (ship.position - runway_pos).length() / 1000.0;
            let cue_color = SKYBLUE.lerp(WHITE, (ship.position.y * 0.05).sin().abs());
            let label = format!("[ RWY 01 | {:.1}km ]", dist_km);
            draw_text(label, screen_pos.x as i32 - 45, screen_pos.y as i32 - 18, 13, cue_color);
            draw_rectangle_lines(
                screen_pos.x as i32 - 12,
                screen_pos.y as i32 - 12,
                24,
                24,
                cue_color,
            );
        }
    }

    fn draw_artificial_horizon(ship: &SpaceshipPhysics, cx: f32, cy: f32) {
        unsafe {
            // Pitch angle: angle of forward vector relative to horizontal plane
            let pitch = ship.forward.y.asin(); // radians (-PI/2 to PI/2)

            // Roll angle: angle of ship right vector relative to world horizontal
            let roll = (ship.right.y).atan2((ship.up.y).max(0.0001));

            let cos_r = roll.cos();
            let sin_r = roll.sin();

            let horizon_len = 90.0;
            let pitch_pixels_per_rad = 350.0;
            let pitch_offset = -pitch * pitch_pixels_per_rad;

            // Horizon line (tilted by roll, shifted by pitch)
            let hx0 = cx - cos_r * horizon_len - sin_r * pitch_offset;
            let hy0 = cy - sin_r * horizon_len + cos_r * pitch_offset;
            let hx1 = cx + cos_r * horizon_len - sin_r * pitch_offset;
            let hy1 = cy + sin_r * horizon_len + cos_r * pitch_offset;

            DrawLineEx(
                Vector2 { x: hx0, y: hy0 },
                Vector2 { x: hx1, y: hy1 },
                2.0,
                Color { r: 50, g: 220, b: 90, a: 220 },
            );

            // Pitch ladder rungs (+10°, +20°, -10°, -20°)
            for deg in [-20.0, -10.0, 10.0, 20.0] {
                let rad = deg * PI / 180.0;
                let offset = -(pitch - rad) * pitch_pixels_per_rad;
                if offset.abs() < 140.0 {
                    let rung_w = 30.0;
                    let rx0 = cx - cos_r * rung_w - sin_r * offset;
                    let ry0 = cy - sin_r * rung_w + cos_r * offset;
                    let rx1 = cx + cos_r * rung_w - sin_r * offset;
                    let ry1 = cy + sin_r * rung_w + cos_r * offset;

                    let rung_color = if deg > 0.0 {
                        Color { r: 80, g: 200, b: 240, a: 160 } // sky ladder
                    } else {
                        Color { r: 230, g: 140, b: 60, a: 160 } // ground ladder
                    };

                    DrawLineEx(Vector2 { x: rx0, y: ry0 }, Vector2 { x: rx1, y: ry1 }, 1.5, rung_color);
                }
            }

            // Fixed aircraft reticle / crosshair at center screen
            let reticle_color = GREEN;
            DrawLineEx(Vector2 { x: cx - 18.0, y: cy }, Vector2 { x: cx - 6.0, y: cy }, 2.0, reticle_color);
            DrawLineEx(Vector2 { x: cx + 6.0, y: cy }, Vector2 { x: cx + 18.0, y: cy }, 2.0, reticle_color);
            DrawLineEx(Vector2 { x: cx, y: cy - 10.0 }, Vector2 { x: cx, y: cy - 4.0 }, 2.0, reticle_color);
            DrawCircle(cx as i32, cy as i32, 2.0, reticle_color);
        }
    }

    fn draw_flight_path_marker(ship: &SpaceshipPhysics, cx: f32, cy: f32) {
        unsafe {
            if ship.airspeed > 5.0 {
                let rel_x = ship.velocity.dot(ship.right) / ship.airspeed;
                let rel_y = ship.velocity.dot(ship.up) / ship.airspeed;

                let fpm_x = (cx + rel_x * 250.0).clamp(cx - 150.0, cx + 150.0);
                let fpm_y = (cy - rel_y * 250.0).clamp(cy - 120.0, cy + 120.0);

                let marker_color = YELLOW;
                DrawCircleLines(fpm_x as i32, fpm_y as i32, 7.0, marker_color);
                DrawLine(fpm_x as i32 - 12, fpm_y as i32, fpm_x as i32 - 7, fpm_y as i32, marker_color);
                DrawLine(fpm_x as i32 + 7, fpm_y as i32, fpm_x as i32 + 12, fpm_y as i32, marker_color);
                DrawLine(fpm_x as i32, fpm_y as i32 - 12, fpm_x as i32, fpm_y as i32 - 7, marker_color);
            }
        }
    }

    fn draw_compass_frame(cx: f32, cy: f32) {
        unsafe {
            let box_w = 180.0;
            let box_h = 32.0;
            DrawRectangle(
                (cx - box_w * 0.5) as i32,
                (cy - box_h * 0.5) as i32,
                box_w as i32,
                box_h as i32,
                Color { r: 15, g: 25, b: 20, a: 200 },
            );
            DrawRectangleLines(
                (cx - box_w * 0.5) as i32,
                (cy - box_h * 0.5) as i32,
                box_w as i32,
                box_h as i32,
                DARKGREEN,
            );

            // Compass indicator tick
            DrawTriangle(
                Vector2 { x: cx, y: cy + 18.0 },
                Vector2 { x: cx - 6.0, y: cy + 26.0 },
                Vector2 { x: cx + 6.0, y: cy + 26.0 },
                GREEN,
            );
        }
    }

    fn draw_tape_frames(_cx: f32, cy: f32, sw: f32) {
        unsafe {
            // Speed tape background frame (Left)
            let spd_x = 65.0;
            DrawRectangle(
                (spd_x - 45.0) as i32,
                (cy - 85.0) as i32,
                90,
                170,
                Color { r: 15, g: 25, b: 20, a: 180 },
            );
            DrawRectangleLines(
                (spd_x - 45.0) as i32,
                (cy - 85.0) as i32,
                90,
                170,
                DARKGREEN,
            );

            // Altitude tape background frame (Right)
            let alt_x = sw - 65.0;
            DrawRectangle(
                (alt_x - 50.0) as i32,
                (cy - 85.0) as i32,
                100,
                170,
                Color { r: 15, g: 25, b: 20, a: 180 },
            );
            DrawRectangleLines(
                (alt_x - 50.0) as i32,
                (cy - 85.0) as i32,
                100,
                170,
                DARKGREEN,
            );
        }
    }
}
