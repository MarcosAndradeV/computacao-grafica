use muxapp::muxui::Vector3;

#[derive(Debug, Clone, Copy, Default, PartialEq)]
pub struct FlightControlInput {
    pub pitch: f32,          // -1.0 (down) to +1.0 (up)
    pub roll: f32,           // -1.0 (left) to +1.0 (right)
    pub yaw: f32,            // -1.0 (left) to +1.0 (right)
    pub throttle_delta: f32, // +/- delta to throttle
    pub afterburner: bool,
    pub airbrake: bool,
    pub vtol_up: f32,        // 0.0 to 1.0 vertical thruster
    pub vtol_down: f32,
    pub gear_toggle: bool,
    pub hover_mode_toggle: bool,
}

#[derive(Debug, Clone)]
pub struct SpaceshipPhysics {
    // Spatial State
    pub position: Vector3,
    pub velocity: Vector3,
    pub forward: Vector3,
    pub right: Vector3,
    pub up: Vector3,

    // Angular rates
    pub angular_velocity: Vector3, // pitch, roll, yaw rates

    // Flight Controls & Engine
    pub throttle: f32,        // 0.0 to 1.0
    pub max_thrust: f32,      // Newtons / mass (acceleration)
    pub boost_multiplier: f32,
    pub airbrake: bool,
    pub gear_down: bool,
    pub hover_mode: bool,

    // Aerodynamic Constants
    pub mass: f32,
    pub wing_area: f32,
    pub lift_coefficient: f32,
    pub drag_coefficient: f32,
    pub stall_speed: f32,

    // Telemetry outputs
    pub airspeed: f32,
    pub vertical_speed: f32,
    pub altitude_msl: f32,
    pub altitude_agl: f32,
    pub g_force: f32,
    pub is_stalling: bool,
    pub on_ground: bool,
}

impl Default for SpaceshipPhysics {
    fn default() -> Self {
        let mut ship = Self {
            position: Vector3::new(0.0, 120.0, 0.0), // Start mid-air at 120m
            velocity: Vector3::new(0.0, 0.0, 65.0),  // Initial cruise speed forward (along +Z)
            forward: Vector3::new(0.0, 0.0, 1.0),
            right: Vector3::new(1.0, 0.0, 0.0),
            up: Vector3::new(0.0, 1.0, 0.0),
            angular_velocity: Vector3::zero(),
            throttle: 0.65, // 65% cruise throttle
            max_thrust: 38.0,
            boost_multiplier: 1.6,
            airbrake: false,
            gear_down: false,
            hover_mode: false,
            mass: 1200.0,
            wing_area: 18.0,
            lift_coefficient: 0.52,
            drag_coefficient: 0.045,
            stall_speed: 28.0,
            airspeed: 65.0,
            vertical_speed: 0.0,
            altitude_msl: 120.0,
            altitude_agl: 120.0,
            g_force: 1.0,
            is_stalling: false,
            on_ground: false,
        };
        ship.normalize_basis();
        ship
    }
}

impl SpaceshipPhysics {
    /// Re-orthonormalizes basis vectors to prevent drift.
    pub fn normalize_basis(&mut self) {
        self.forward = self.forward.normalize();
        self.right = self.forward.cross(self.up).normalize();
        self.up = self.right.cross(self.forward).normalize();
    }

    /// Rotate orientation using pitch, roll, yaw rates.
    pub fn rotate(&mut self, pitch: f32, roll: f32, yaw: f32) {
        // Pitch rotates around Right axis
        if pitch.abs() > 0.00001 {
            self.forward = rotate_around_axis(self.forward, self.right, pitch);
            self.up = rotate_around_axis(self.up, self.right, pitch);
        }

        // Roll rotates around Forward axis
        if roll.abs() > 0.00001 {
            self.right = rotate_around_axis(self.right, self.forward, roll);
            self.up = rotate_around_axis(self.up, self.forward, roll);
        }

        // Yaw rotates around Up axis
        if yaw.abs() > 0.00001 {
            self.forward = rotate_around_axis(self.forward, self.up, yaw);
            self.right = rotate_around_axis(self.right, self.up, yaw);
        }

        self.normalize_basis();
    }

    /// Primary physics integration step.
    pub fn step(&mut self, input: &FlightControlInput, ground_height: f32, dt: f32) {
        if dt <= 0.0 {
            return;
        }

        // 1. Throttle adjustments
        self.throttle = (self.throttle + input.throttle_delta * dt).clamp(0.0, 1.0);
        if input.airbrake {
            self.airbrake = !self.airbrake;
        }
        if input.gear_toggle {
            self.gear_down = !self.gear_down;
        }
        if input.hover_mode_toggle {
            self.hover_mode = !self.hover_mode;
        }

        // 2. Control surface rotation authority (scales with dynamic pressure / airspeed)
        let control_authority = if self.hover_mode {
            1.0
        } else {
            (self.airspeed / 50.0).clamp(0.2, 1.3)
        };

        let pitch_rate = input.pitch * 1.6 * control_authority * dt;
        let roll_rate = input.roll * 2.8 * control_authority * dt;
        let yaw_rate = input.yaw * 0.9 * control_authority * dt;

        self.angular_velocity = Vector3::new(pitch_rate, roll_rate, yaw_rate);
        self.rotate(pitch_rate, roll_rate, yaw_rate);

        // 3. Atmospheric density calculation (exponential barometric formula)
        let rho_0 = 1.225; // Sea level density kg/m^3
        let scale_height = 4000.0;
        let density = (rho_0 * (-self.position.y / scale_height).exp()).clamp(0.01, 1.3);

        // 4. Calculate relative air velocities
        let vel_mag = self.velocity.length();
        let fwd_speed = self.velocity.dot(self.forward);
        let right_speed = self.velocity.dot(self.right);
        let up_speed = self.velocity.dot(self.up);

        self.airspeed = vel_mag;
        self.vertical_speed = self.velocity.y;
        self.altitude_msl = self.position.y;
        self.altitude_agl = (self.position.y - ground_height).max(0.0);

        // 5. Total Forces calculation
        let mut total_accel = Vector3::zero();

        // Gravity: Downward towards planetary center
        let g = 9.81;
        total_accel.y -= g;

        // Engine Thrust
        let mut thrust_mag = self.throttle * self.max_thrust;
        if input.afterburner && self.throttle > 0.8 {
            thrust_mag *= self.boost_multiplier;
        }
        total_accel = total_accel + self.forward * thrust_mag;

        // VTOL / Hover Thrusters
        if input.vtol_up > 0.0 {
            total_accel = total_accel + self.up * (input.vtol_up * 18.0);
        }
        if input.vtol_down > 0.0 {
            total_accel = total_accel - self.up * (input.vtol_down * 15.0);
        }
        if self.hover_mode {
            // Hover autopilot counteracting gravity and stabilizing descent
            let hover_target_accel = (g - self.velocity.y * 1.5).clamp(0.0, g * 2.0);
            total_accel = total_accel + self.up * hover_target_accel;
        }

        // Aerodynamics: Lift & Drag
        if !self.on_ground {
            // Lift: Generated by forward velocity along the ship's local Up axis
            let dynamic_pressure = 0.5 * density * fwd_speed.max(0.0).powi(2);
            let stall_factor = if fwd_speed < self.stall_speed {
                self.is_stalling = true;
                (fwd_speed / self.stall_speed).max(0.0).powi(2)
            } else {
                self.is_stalling = false;
                1.0
            };

            let lift_force = dynamic_pressure * self.wing_area * self.lift_coefficient * stall_factor;
            let lift_accel = self.up * (lift_force / self.mass);
            total_accel = total_accel + lift_accel;

            // Forward Parasitic Drag
            let mut drag_coeff = self.drag_coefficient;
            if self.airbrake {
                drag_coeff *= 3.5;
            }
            if self.gear_down {
                drag_coeff *= 1.8;
            }

            let fwd_drag_mag = dynamic_pressure * self.wing_area * drag_coeff;
            let fwd_drag = -self.forward * (fwd_drag_mag / self.mass);
            total_accel = total_accel + fwd_drag;

            // Lateral and vertical crossflow damping (planes don't drift sideways in atmosphere)
            let crossflow_damping = 0.5 * density * 6.0;
            let lateral_drag = -self.right * (right_speed * crossflow_damping);
            let vertical_drag = -self.up * (up_speed * crossflow_damping);
            total_accel = total_accel + lateral_drag + vertical_drag;
        }

        // Calculate perceived G-force (total non-gravitational acceleration / g)
        let aero_and_thrust = Vector3::new(total_accel.x, total_accel.y + g, total_accel.z);
        self.g_force = (aero_and_thrust.length() / g).clamp(0.0, 9.0);

        // 6. Velocity & Position Integration
        self.velocity = self.velocity + total_accel * dt;

        // 7. Ground collision & landing mechanics
        let landing_clearance = 1.2;
        if self.position.y <= ground_height + landing_clearance {
            self.position.y = ground_height + landing_clearance;
            self.on_ground = true;

            // If landing gear is down and vertical touchdown is gentle
            if self.gear_down && self.velocity.y.abs() < 7.0 && self.up.y > 0.6 {
                // Ground wheel rolling friction
                self.velocity.y = 0.0;
                let ground_friction: f32 = if self.airbrake { 0.85 } else { 0.97 };
                self.velocity.x *= ground_friction.powf(dt * 60.0);
                self.velocity.z *= ground_friction.powf(dt * 60.0);

                // Align ship attitude slightly with level ground
                self.up.x *= 0.95;
                self.up.z *= 0.95;
                self.up.y = 1.0;
                self.normalize_basis();
            } else {
                // Hard impact / bounce
                self.velocity.y = self.velocity.y.abs() * 0.3;
                self.velocity.x *= 0.7;
                self.velocity.z *= 0.7;
            }
        } else {
            self.on_ground = false;
        }

        self.position = self.position + self.velocity * dt;
    }
}

/// Rotates vector `v` around unit axis `axis` by `angle` radians (Rodrigues' rotation formula).
pub fn rotate_around_axis(v: Vector3, axis: Vector3, angle: f32) -> Vector3 {
    let cos_theta = angle.cos();
    let sin_theta = angle.sin();
    let cross_prod = axis.cross(v);
    let dot_prod = axis.dot(v);

    v * cos_theta + cross_prod * sin_theta + axis * (dot_prod * (1.0 - cos_theta))
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_basis_vectors_orthonormal() {
        let mut ship = SpaceshipPhysics::default();
        ship.rotate(0.5, 0.3, -0.2);

        let dot_fr = ship.forward.dot(ship.right);
        let dot_fu = ship.forward.dot(ship.up);
        let dot_ru = ship.right.dot(ship.up);

        assert!(dot_fr.abs() < 1e-4, "Forward and Right must be orthogonal");
        assert!(dot_fu.abs() < 1e-4, "Forward and Up must be orthogonal");
        assert!(dot_ru.abs() < 1e-4, "Right and Up must be orthogonal");

        assert!((ship.forward.length() - 1.0).abs() < 1e-4);
        assert!((ship.right.length() - 1.0).abs() < 1e-4);
        assert!((ship.up.length() - 1.0).abs() < 1e-4);
    }

    #[test]
    fn test_stall_at_low_speed() {
        let mut ship = SpaceshipPhysics::default();
        ship.velocity = Vector3::new(0.0, 0.0, 10.0); // below 28 m/s stall threshold
        let input = FlightControlInput {
            pitch: 0.0,
            roll: 0.0,
            yaw: 0.0,
            throttle_delta: 0.0,
            afterburner: false,
            airbrake: false,
            vtol_up: 0.0,
            vtol_down: 0.0,
            gear_toggle: false,
            hover_mode_toggle: false,
        };

        ship.step(&input, 0.0, 0.016);
        assert!(ship.is_stalling, "Ship should be stalling at 10 m/s");
    }

    #[test]
    fn test_throttle_clamp() {
        let mut ship = SpaceshipPhysics::default();
        let mut input = FlightControlInput {
            pitch: 0.0,
            roll: 0.0,
            yaw: 0.0,
            throttle_delta: 10.0,
            afterburner: false,
            airbrake: false,
            vtol_up: 0.0,
            vtol_down: 0.0,
            gear_toggle: false,
            hover_mode_toggle: false,
        };

        ship.step(&input, 0.0, 1.0);
        assert!(ship.throttle <= 1.0);

        input.throttle_delta = -10.0;
        ship.step(&input, 0.0, 1.0);
        assert!(ship.throttle >= 0.0);
    }

    #[test]
    fn test_flight_control_input_default() {
        let input = FlightControlInput::default();
        assert_eq!(input.pitch, 0.0);
        assert_eq!(input.roll, 0.0);
        assert_eq!(input.yaw, 0.0);
        assert_eq!(input.throttle_delta, 0.0);
        assert!(!input.afterburner);
        assert!(!input.airbrake);
        assert_eq!(input.vtol_up, 0.0);
        assert_eq!(input.vtol_down, 0.0);
    }
}
