use muxapp::muxui::{d3::*, *};
use std::f32::consts::PI;

const GRID_SIZE: i32 = 60;
const CELL_SIZE: f32 = 25.0;

pub struct PlanetaryTerrain {
    pub grid_size: i32,
    pub cell_size: f32,
    pub water_level: f32,
}

impl Default for PlanetaryTerrain {
    fn default() -> Self {
        Self {
            grid_size: GRID_SIZE,
            cell_size: CELL_SIZE,
            water_level: 0.0,
        }
    }
}

impl PlanetaryTerrain {
    /// Returns elevation at (x, z) coordinates.
    pub fn get_height(&self, x: f32, z: f32) -> f32 {
        // Flat runway strip: centered at x=0, stretching from z=-250 to z=250
        let runway_w = 25.0;
        let runway_l = 300.0;
        if x.abs() < runway_w && z.abs() < runway_l {
            return 0.5; // elevated runway surface
        }

        // Smooth transition away from runway
        let dx = (x.abs() - runway_w).max(0.0) / 70.0;
        let dz = (z.abs() - runway_l).max(0.0) / 90.0;
        let dist_from_runway = (dx * dx + dz * dz).sqrt().clamp(0.0, 1.0);

        // Procedural multi-frequency landscape
        let freq1 = 0.003;
        let h1 = (x * freq1).sin() * (z * freq1).cos() * 85.0;

        let freq2 = 0.008;
        let h2 = ((x * 0.7 + z * 0.7) * freq2).sin() * 35.0;

        let freq3 = 0.02;
        let h3 = ((x * 1.3 - z * 0.9) * freq3).cos() * 12.0;

        // Base ground elevation (average rolling terrain above water)
        let base_ground = 12.0;
        let raw_h = base_ground + h1 + h2 + h3;
        0.5 + (raw_h - 0.5) * dist_from_runway
    }

    /// Returns the effective physical surface elevation (ground or ocean surface).
    ///
    /// Used by flight physics and collision detection to ensure aircraft treat the ocean
    /// surface (sea level) as a solid contact boundary rather than sinking to the seabed.
    pub fn get_surface_height(&self, x: f32, z: f32) -> f32 {
        self.get_height(x, z).max(self.water_level)
    }

    /// Draws the planetary landscape, runway, ocean, and atmosphere using batch-safe MUX 0.3.0 chunking.
    pub fn draw(&self, d3: &Render3DContext, center_x: f32, center_z: f32) {
        // Directional sunlight vector from sky
        let sun_dir = Vector3::new(0.4, 0.85, 0.35).normalize();

        // Snap center to grid steps to prevent geometry jittering
        let snap_x = (center_x / self.cell_size).floor() * self.cell_size;
        let snap_z = (center_z / self.cell_size).floor() * self.cell_size;

        let half_grid = self.grid_size / 2;
        let start_x = snap_x - (half_grid as f32) * self.cell_size;
        let start_z = snap_z - (half_grid as f32) * self.cell_size;

        let mut terrain_vertices =
            Vec::with_capacity((self.grid_size * self.grid_size * 6) as usize);

        // Build terrain mesh (60x60 = 21,600 vertices safely chunked by draw_trimesh_buffered)
        for i in 0..self.grid_size {
            let x0 = start_x + (i as f32) * self.cell_size;
            let x1 = x0 + self.cell_size;

            for j in 0..self.grid_size {
                let z0 = start_z + (j as f32) * self.cell_size;
                let z1 = z0 + self.cell_size;

                let y00 = self.get_height(x0, z0);
                let y10 = self.get_height(x1, z0);
                let y01 = self.get_height(x0, z1);
                let y11 = self.get_height(x1, z1);

                let p00 = Vector3::new(x0, y00, z0);
                let p10 = Vector3::new(x1, y10, z0);
                let p01 = Vector3::new(x0, y01, z1);
                let p11 = Vector3::new(x1, y11, z1);

                // --- Triangle 1: p00 -> p01 -> p10 (Counter-Clockwise, normal points +Y) ---
                let e1 = p01 - p00;
                let e2 = p10 - p00;
                let n1 = e1.cross(e2).normalize();
                let light1 = 0.32 + (n1.dot(sun_dir)).max(0.0) * 0.68;
                let c1 = Self::shaded_color((y00 + y01 + y10) / 3.0, light1);

                terrain_vertices.push(Vertex3D::new(p00, c1).with_normal(n1));
                terrain_vertices.push(Vertex3D::new(p01, c1).with_normal(n1));
                terrain_vertices.push(Vertex3D::new(p10, c1).with_normal(n1));

                // --- Triangle 2: p10 -> p01 -> p11 (Counter-Clockwise, normal points +Y) ---
                let e3 = p01 - p10;
                let e4 = p11 - p10;
                let n2 = e3.cross(e4).normalize();
                let light2 = 0.32 + (n2.dot(sun_dir)).max(0.0) * 0.68;
                let c2 = Self::shaded_color((y10 + y01 + y11) / 3.0, light2);

                terrain_vertices.push(Vertex3D::new(p10, c2).with_normal(n2));
                terrain_vertices.push(Vertex3D::new(p01, c2).with_normal(n2));
                terrain_vertices.push(Vertex3D::new(p11, c2).with_normal(n2));
            }
        }

        // Draw terrain safely chunked
        d3.draw_trimesh_buffered(&terrain_vertices);

        // Draw Ocean / Water plane (with CCW winding)
        self.draw_ocean(d3, center_x, center_z);

        // Draw Runway markings & lights (with CCW winding)
        self.draw_runway(d3);

        // Draw Distant Horizon & Sky rings
        self.draw_atmosphere(d3, center_x, center_z);
    }

    fn shaded_color(h: f32, light: f32) -> Color {
        let base = if h < -25.0 {
            Color {
                r: 14,
                g: 28,
                b: 46,
                a: 255,
            } // Deep abyssal ocean floor
        } else if h < -8.0 {
            Color {
                r: 20,
                g: 48,
                b: 70,
                a: 255,
            } // Continental shelf seabed
        } else if h < 0.0 {
            Color {
                r: 32,
                g: 78,
                b: 92,
                a: 255,
            } // Submerged coastal shoals / reef
        } else if h < 3.0 {
            Color {
                r: 215,
                g: 200,
                b: 150,
                a: 255,
            } // Sandy shore / beach above water
        } else if h < 30.0 {
            Color {
                r: 45,
                g: 135,
                b: 55,
                a: 255,
            } // Vibrant lowland grass
        } else if h < 60.0 {
            Color {
                r: 95,
                g: 130,
                b: 65,
                a: 255,
            } // Highland plateau
        } else if h < 95.0 {
            Color {
                r: 125,
                g: 120,
                b: 115,
                a: 255,
            } // Rocky mountain slate
        } else {
            Color {
                r: 245,
                g: 248,
                b: 252,
                a: 255,
            } // Snow peak
        };

        Color {
            r: (base.r as f32 * light).clamp(0.0, 255.0) as u8,
            g: (base.g as f32 * light).clamp(0.0, 255.0) as u8,
            b: (base.b as f32 * light).clamp(0.0, 255.0) as u8,
            a: 255,
        }
    }

    fn draw_runway(&self, d3: &Render3DContext) {
        let rw = 22.0;
        let rl = 280.0;
        let y = 0.55;

        let mut runway_vertices = Vec::with_capacity(128);

        // Dark asphalt runway surface (Counter-Clockwise, normal points +Y)
        let asphalt_color = Color {
            r: 40,
            g: 42,
            b: 45,
            a: 255,
        };
        let norm = Vector3::new(0.0, 1.0, 0.0);

        runway_vertices
            .push(Vertex3D::new(Vector3::new(-rw, y, -rl), asphalt_color).with_normal(norm));
        runway_vertices
            .push(Vertex3D::new(Vector3::new(-rw, y, rl), asphalt_color).with_normal(norm));
        runway_vertices
            .push(Vertex3D::new(Vector3::new(rw, y, -rl), asphalt_color).with_normal(norm));

        runway_vertices
            .push(Vertex3D::new(Vector3::new(rw, y, -rl), asphalt_color).with_normal(norm));
        runway_vertices
            .push(Vertex3D::new(Vector3::new(-rw, y, rl), asphalt_color).with_normal(norm));
        runway_vertices
            .push(Vertex3D::new(Vector3::new(rw, y, rl), asphalt_color).with_normal(norm));

        // White Centerline stripes (Counter-Clockwise)
        let stripe_color = Color {
            r: 245,
            g: 245,
            b: 245,
            a: 255,
        };
        let stripe_w = 1.0;
        let stripe_len = 16.0;
        let gap = 16.0;
        let mut z = -rl + 20.0;
        while z < rl - 20.0 {
            let z_next = z + stripe_len;
            let sy = y + 0.05;

            runway_vertices.push(
                Vertex3D::new(Vector3::new(-stripe_w, sy, z), stripe_color).with_normal(norm),
            );
            runway_vertices.push(
                Vertex3D::new(Vector3::new(-stripe_w, sy, z_next), stripe_color).with_normal(norm),
            );
            runway_vertices
                .push(Vertex3D::new(Vector3::new(stripe_w, sy, z), stripe_color).with_normal(norm));

            runway_vertices
                .push(Vertex3D::new(Vector3::new(stripe_w, sy, z), stripe_color).with_normal(norm));
            runway_vertices.push(
                Vertex3D::new(Vector3::new(-stripe_w, sy, z_next), stripe_color).with_normal(norm),
            );
            runway_vertices.push(
                Vertex3D::new(Vector3::new(stripe_w, sy, z_next), stripe_color).with_normal(norm),
            );

            z += stripe_len + gap;
        }

        d3.draw_trimesh_buffered(&runway_vertices);

        // Runway green threshold lights & red end lights
        let mut lz = -rl;
        while lz <= rl {
            let light_color = if lz.abs() > rl - 15.0 { RED } else { LIME };
            d3.draw_sphere(Vector3::new(-rw - 2.5, 0.9, lz), 0.6, light_color);
            d3.draw_sphere(Vector3::new(rw + 2.5, 0.9, lz), 0.6, light_color);
            lz += 25.0;
        }
    }

    fn draw_ocean(&self, d3: &Render3DContext, center_x: f32, center_z: f32) {
        let r = 2000.0;
        let y = self.water_level;
        let ocean_color = Color {
            r: 24,
            g: 90,
            b: 165,
            a: 215,
        };
        let norm = Vector3::new(0.0, 1.0, 0.0);

        let ocean_vertices = [
            Vertex3D::new(Vector3::new(center_x - r, y, center_z - r), ocean_color)
                .with_normal(norm),
            Vertex3D::new(Vector3::new(center_x - r, y, center_z + r), ocean_color)
                .with_normal(norm),
            Vertex3D::new(Vector3::new(center_x + r, y, center_z - r), ocean_color)
                .with_normal(norm),
            Vertex3D::new(Vector3::new(center_x + r, y, center_z - r), ocean_color)
                .with_normal(norm),
            Vertex3D::new(Vector3::new(center_x - r, y, center_z + r), ocean_color)
                .with_normal(norm),
            Vertex3D::new(Vector3::new(center_x + r, y, center_z + r), ocean_color)
                .with_normal(norm),
        ];

        d3.draw_trimesh_buffered(&ocean_vertices);
    }

    fn draw_atmosphere(&self, d3: &Render3DContext, center_x: f32, center_z: f32) {
        // Horizon atmospheric haze dome
        let segments = 36;
        let radius = 1200.0;
        let height = 450.0;

        let mut dome_vertices = Vec::with_capacity(segments * 6);
        let ground_color = Color {
            r: 130,
            g: 180,
            b: 230,
            a: 180,
        };
        let sky_color = Color {
            r: 35,
            g: 75,
            b: 155,
            a: 0,
        };

        for i in 0..segments {
            let a0 = (i as f32 / segments as f32) * 2.0 * PI;
            let a1 = ((i + 1) as f32 / segments as f32) * 2.0 * PI;

            let x0 = center_x + a0.cos() * radius;
            let z0 = center_z + a0.sin() * radius;
            let x1 = center_x + a1.cos() * radius;
            let z1 = center_z + a1.sin() * radius;

            dome_vertices.push(Vertex3D::new(Vector3::new(x0, 0.0, z0), ground_color));
            dome_vertices.push(Vertex3D::new(Vector3::new(x1, 0.0, z1), ground_color));
            dome_vertices.push(Vertex3D::new(Vector3::new(x0, height, z0), sky_color));

            dome_vertices.push(Vertex3D::new(Vector3::new(x1, 0.0, z1), ground_color));
            dome_vertices.push(Vertex3D::new(Vector3::new(x1, height, z1), sky_color));
            dome_vertices.push(Vertex3D::new(Vector3::new(x0, height, z0), sky_color));
        }

        d3.draw_trimesh_buffered(&dome_vertices);
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_runway_elevation_is_flat() {
        let terrain = PlanetaryTerrain::default();
        // Inside runway zone (x in -25..25, z in -300..300)
        assert_eq!(terrain.get_height(0.0, 0.0), 0.5);
        assert_eq!(terrain.get_height(10.0, 100.0), 0.5);
        assert_eq!(terrain.get_height(-10.0, -150.0), 0.5);
    }

    #[test]
    fn test_surface_elevation_non_negative() {
        let terrain = PlanetaryTerrain::default();
        for x in [-500.0, -200.0, 0.0, 300.0, 600.0] {
            for z in [-500.0, -200.0, 0.0, 300.0, 600.0] {
                let h = terrain.get_surface_height(x, z);
                assert!(
                    h >= terrain.water_level,
                    "Surface elevation should not be below water level"
                );
            }
        }
    }

    #[test]
    fn test_seabed_elevation_below_water() {
        let terrain = PlanetaryTerrain::default();
        // Point in ocean valley where h1, h2, h3 sum to negative
        let mut found_seabed = false;
        for x in (-50..50).map(|i| i as f32 * 30.0) {
            for z in (-50..50).map(|i| i as f32 * 30.0) {
                if terrain.get_height(x, z) < terrain.water_level {
                    found_seabed = true;
                    break;
                }
            }
            if found_seabed {
                break;
            }
        }
        assert!(
            found_seabed,
            "Expected ocean basins to descend beneath sea level to avoid coplanar Z-fighting"
        );
    }
}
