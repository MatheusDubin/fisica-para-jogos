# Scene Specification — Ground Truth

> Every value in this document must be reproduced identically in Unity, Unreal Engine, and Godot.  
> If an engine forces a different unit system (e.g., Unreal uses centimeters), convert and note it explicitly.

---

## Global Physics Settings

| Parameter | Value | Notes |
|---|---|---|
| Gravity | `(0, -9.81, 0) m/s²` | Unreal: `(0, 0, -981) cm/s²` |
| Fixed Timestep | `0.02 s (50 Hz)` | Unity: `Time.fixedDeltaTime = 0.02` · Unreal: `Fixed Framerate = 50` · Godot: `physics/common/physics_ticks_per_second = 50` |
| Sleep Linear Threshold | `0.05 m/s` | Unity: `Physics.sleepThreshold` · Unreal: `SleepThresholdMultiplier` (see Dicas) · Godot: `ProjectSettings > physics/3d/sleep_threshold_linear` |
| Sleep Angular Threshold | `0.05 rad/s` | Same sources as above |
| Solver Iterations | Default (do not override) | Changing this would alter the solver behavior we are benchmarking |

---

## Shared Object Properties

All rigid bodies in both scenarios use these values unless otherwise noted.

| Property | Value |
|---|---|
| Shape | Box (cube) |
| Collider size | `1 m × 1 m × 1 m` (Unreal: `100 × 100 × 100 cm`) |
| Visual mesh | Same primitive cube (no custom mesh) |
| Mass | `1 kg` |
| Static friction | `0.6` |
| Dynamic friction | `0.4` |
| Restitution (bounciness) | `0.0` |
| Initial linear velocity | `(0, 0, 0)` |
| Initial angular velocity | `(0, 0, 0)` |

---

## Scenario 1 — The Tower

### Layout

Boxes are stacked in a single vertical column, centered at the world origin on the XZ plane.

| Parameter | Value |
|---|---|
| Column center (XZ) | `(0, 0, 0)` |
| First box bottom Y | `0.5 m` (so its center is at Y = 0.5) |
| Vertical spacing | `1.0 m` center-to-center (boxes flush, no gap) |
| Box N center Y | `N × 1.0 m - 0.5 m` (N starts at 1) |
| Floor | Static plane/box at Y = 0, infinite or large enough (≥ 20 m × 20 m) |
| Floor friction (static) | `0.8` |
| Floor restitution | `0.0` |

### Test Variants

| Variant | Block count | Max expected height |
|---|---|---|
| T-50 | 50 | 50 m |
| T-100 | 100 | 100 m |
| T-200 | 200 | 200 m |

### Measurement

- Start timer when simulation begins (`t = 0`)
- Stop timer when **all** boxes report sleeping / velocity below threshold for **1 full second**
- Record: time-to-sleep (s), whether the stack collapsed (bool), subjective jitter rating (none / light / heavy)

---

## Scenario 2 — Body Rain

### Funnel / Container Geometry

A static funnel that forces objects to pile up at a central exit point.

| Parameter | Value |
|---|---|
| Funnel shape | 4 angled static walls forming an inverted pyramid |
| Funnel top opening | `40 m × 40 m` (Unreal: `4000 × 4000 cm`) |
| Funnel bottom opening | `4 m × 4 m` (Unreal: `400 × 400 cm`) |
| Funnel height | `20 m` (Unreal: `2000 cm`) |
| Wall thickness | `0.5 m` (static, no physics) |
| Wall restitution | `0.0` |
| Wall friction (static) | `0.6` |
| Floor below funnel | Static plane at Y = -25 m, size ≥ 60 m × 60 m |

### Spawn Zone

Objects are spawned inside the funnel top opening.

| Parameter | Value |
|---|---|
| Spawn area | `35 m × 35 m` centered at `(0, 22, 0)` (2 m above funnel rim) |
| Spawn pattern | Random uniform XZ within spawn area |
| Spawn Y jitter | Random offset in `[0, 5] m` to avoid simultaneous spawning of all objects |
| Spawn mode | All objects instantiated at `t = 0` with their Y offset as initial position |

### Test Variants

| Variant | Object count |
|---|---|
| R-1k | 1,000 |
| R-5k | 5,000 |
| R-10k | 10,000 |

### Measurement Window

- Begin recording **immediately after all objects are spawned**
- Record for **30 seconds** of simulation time
- Sample Physics Step Time and FPS **every fixed tick** during the window
- Report: mean Physics Step Time (ms), mean FPS, min FPS

---

## Coordinate System Notes

| Engine | Up axis | Unit | Conversion factor |
|---|---|---|---|
| Unity | Y | meters | 1× |
| Unreal Engine | Z | centimeters | multiply all meter values by 100; swap Y↔Z in spawn coords |
| Godot | Y | meters | 1× |

For Unreal, the Tower column center becomes `(0, 0, 50 cm)` (first box bottom at Z = 50 cm), and spawn Y becomes Z.

---

## Checklist Before Running

- [ ] Gravity confirmed for engine (see table above)
- [ ] Fixed timestep = 0.02 s confirmed
- [ ] Sleep thresholds set (linear 0.05, angular 0.05)
- [ ] All boxes: mass = 1 kg, friction static 0.6 / dynamic 0.4, restitution 0.0
- [ ] Tower spacing verified (1 m center-to-center)
- [ ] Funnel dimensions verified
- [ ] Spawn zone dimensions and Y-jitter verified
- [ ] Measurement window: 30 s for Scenario 2, sleep-detection for Scenario 1
