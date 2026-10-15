# MUX Framework Developer Experience & Friction Report

This document records Developer Experience (DX) observations, friction points, and recommendations gathered while building the planetary flight simulation and multi-scene architecture in `computacao-grafica` using **MUX 0.2.0**.

---

## 1. Summary of Improvements in MUX 0.2.0

The MUX 0.2.0 release resolved major initial friction points:
- **3D RAII Scope:** `muxui::d3::draw_3d(camera, |d3| { ... })` and `d3.push_matrix(|ctx| { ... })` replaced manual `unsafe { BeginMode3D/EndMode3D }`.
- **Vector Math Operators:** Operator overloading on `Vector3` (`+`, `-`, `*`, scalar `*`, `/`) and geometric utilities (`dot`, `cross`, `normalize`, `distance`, `lerp`) eliminated awkward manual component math.
- **Scene Router & Actions:** First-class `Scene<Context>` trait and `SceneRouter` enabled modal overlays (Pause menu) and smooth scene transitions (`Switch`, `Push`, `Pop`).
- **Input System:** Semantic action and axis mappings (`register_action`, `register_axis`) decoupled flight mechanics from hardcoded key bindings.

---

---

## 2. Friction Points Identified & Resolved in MUX 0.3.0

All 4 major friction points identified below have been **fully resolved** in MUX 0.3.0:

### A. 3D Render Batch Limits & Raw `rlgl` Safety [RESOLVED in 0.3.0]
- **Friction:** Procedural meshes (>8,192 vertices) caused silent geometry drops due to Raylib's internal batch limit.
- **Resolution in MUX 0.3.0:** Added `Vertex3D` and `d3.draw_trimesh_buffered(&vertices)`. MUX now automatically chunks vertices into safe 4,095-vertex slices and handles batch checking.

### B. Safe Text FFI & 3D Screen Projection [RESOLVED in 0.3.0]
- **Friction:** Passing `&str` to C text APIs caused memory smearing without explicit `\0`, and projecting 3D waypoints to the 2D HUD required manual trigonometry.
- **Resolution in MUX 0.3.0:**
  1. `draw_text` in `raylib-rs` now accepts `impl AsRef<str>` directly with automatic null termination.
  2. `muxutils::get_virtual_world_to_screen(camera, position)` converts 3D world coordinates to 2D virtual canvas positions with frustum culling.

### C. Mouse Delta Integration with Input Axis [RESOLVED in 0.3.0]
- **Friction:** Continuous analog axes required manually polling and scaling mouse delta on top of keyboard axes.
- **Resolution in MUX 0.3.0:** Introduced `MouseAxis` (X, Y, Wheel) and `App::register_mouse_axis(name, axis, sensitivity)`. `Input::get_axis(name)` seamlessly sums keyboard discrete contributions and virtual mouse movements.

### D. Dynamic GSS Classes [RESOLVED in 0.3.0]
- **Friction:** Dynamically toggling colors or sizes on HUD badges required manual code interpolation or duplicate GSS elements.
- **Resolution in MUX 0.3.0:** Added `.with_class(class)` and `.set_class(class)` to `TextElement`, `Button`, and `RectangleElement`, resolved via hierarchical selectors (`element.class.property`).

