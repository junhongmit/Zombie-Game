# Character Rig Asset Spec

This file defines the intended asset contract for aim-driven upper-body character rendering.

The goal is to let gameplay code own the pose while art owns the visual parts.

## Design Intent

The character should be split into:

- lower body animation
- upper body rig parts
- weapon sprite

Recommended first rig scope:

- `head`
- `torso`
- `front_upper_arm`
- `front_forearm`
- `back_upper_arm`
- `back_forearm`

This is enough to support:

- 360-degree aiming
- double-arm IK
- different jackets / armor variants
- weapon grip metadata per gun

## File Layout

Recommended files:

```text
assets/characters/
  man1.png
  man1.sheet
  man1_rig.png
  man1_rig.json
```

- `man1.png` / `man1.sheet`
  lower-body or legacy full-body animation reference
- `man1_rig.png`
  upper-body modular parts sheet
- `man1_rig.json`
  per-part rectangles, pivots, anchors, and optional pose correction data

## Core Rule

The runtime should solve pose first, then place art:

1. solve torso / head / shoulder positions
2. solve weapon rear grip / front grip
3. solve front and back arm IK
4. render body parts into those solved transforms

Art should not encode one fixed aiming angle.

## Coordinate Spaces

Use three spaces:

1. `sheet space`
   pixel coordinates inside `*_rig.png`
2. `part local space`
   coordinates relative to one part sprite and its pivot
3. `rig space`
   world/logical pose points produced by the aiming rig

`solver_length` values live in `rig space`, not in sheet pixels.
That means they use the same gameplay-world units as player position, collision, and IK targets.

When converting authored sprite anchors into solver positions:

- measure anchors in sheet-space pixels
- subtract the part pivot to get sprite-local offsets
- convert sprite `+Y down` into rig `+Y up`
- scale those offsets by the current torso pixel-to-world ratio before feeding IK

That last step matters: authored pixels and gameplay-world units are not interchangeable.

## Part Definition

Each part should define:

- `frame`
  source rectangle in sheet space
- `pivot`
  rotation pivot in sheet-space pixels
- `anchor`
  semantic rig attachment point
- `z_order`
  layering hint
- `solver_length` (optional)
  IK bone length in gameplay-world units

Example:

```json
{
  "frame": { "x": 0, "y": 0, "w": 18, "h": 24 },
  "pivot": { "x": 9, "y": 3 },
  "anchor": "torso_top",
  "z_order": 20,
  "solver_length": 8.5
}

At load time, runtime code converts this sheet-space pivot into part-local space by
subtracting `frame.x` / `frame.y`. This keeps authoring easy in Aseprite while
keeping rendering math simple.
```

## Anchors

Suggested semantic anchors:

- `head_center`
- `torso_base`
- `torso_top`
- `front_shoulder`
- `back_shoulder`
- `front_elbow`
- `back_elbow`
- `rear_grip`
- `front_grip`

Most parts only need one anchor because their pose is derived from the rig transform.

## Torso Guidance

For torso bending, prefer this order:

1. rigid rotation
2. tiny non-uniform scale
3. only later, if needed, segmented torso deformation

Do not start with arbitrary local warp.

Why:

- strong local warp breaks pixel clusters
- it causes shimmer and muddy edges under animation
- it makes clothing variants harder to swap cleanly

Recommended first-pass torso approach:

- one torso sprite
- rotate it with the upper-body aim pose
- allow only very mild scale adjustment at extreme up/down aim:
  - `scale_x`: `0.98` to `1.02`
  - `scale_y`: `0.96` to `1.04`

If more bend is needed later, split torso into:

- `chest`
- `abdomen/pelvis`

That is usually better than warping one pixel-art torso aggressively.

## Arm Guidance

For each arm segment:

- author a relaxed neutral pose
- keep cylindrical sleeve shapes fairly symmetric
- leave enough outline thickness that rotation does not collapse the silhouette
- treat art length and `solver_length` as separate knobs

Recommended arrangement:

- upper arms attach at shoulder pivots
- forearms attach at elbow pivots
- hands may be omitted in first pass if forearm end is visually acceptable

If generated art is a bit too long or too short, prefer tweaking `solver_length`
first instead of forcing gameplay to inherit the exact sprite proportion.

## Weapon Points

Weapon metadata should define points in the weapon's own right-facing local frame:

- `rear_wrist`
- `front_wrist`
- `muzzle`

These are measured in weapon sprite pixels inside the right-facing half-frame.
At runtime they are mirrored automatically for left-facing aim.

If a weapon uses a dedicated held sprite such as `hold_glock.png`, prefer a sibling
metadata file such as `hold_glock.json` to store:

- `rear_wrist`
- `front_wrist`
- `muzzle`
- `mirrored_pair_sprite`

This keeps grip tuning close to the held art instead of burying it in the global catalog.

## Pose Tuning

The current upper-body solver uses a few gameplay constants to keep weapons feeling
"comfortable" instead of fully straight-armed:

- `kRigShortWeaponGripSpanThreshold`
  separates compact weapons (pistols) from long guns by comparing weapon grip spacing
- `kRigDesiredReachRatioShortWeapon`
  target fraction of total arm reach used for compact weapons; lower means more elbow bend
- `kRigDesiredReachRatioLongWeapon`
  target fraction of total arm reach used for long guns; higher means a more extended hold
- `kRigMinMuzzleDistanceShortWeapon`
  prevents pistols from collapsing back into the face
- `kRigMinMuzzleDistanceLongWeapon`
  same idea for rifles and other longer weapons
- `kRigBaseMuzzleDistanceShortWeapon`
  initial pistol placement along the aim line before the comfort solver nudges it
- `kRigBaseMuzzleDistanceLongWeapon`
  initial long-gun placement along the aim line before the comfort solver nudges it

In plain language:

- reach ratio controls elbow openness
- min muzzle distance prevents self-intersection
- base muzzle distance sets the neutral "how close to the body does this weapon sit" feel

If the pose looks too rigid:

- lower the desired reach ratio
- lower the base muzzle distance slightly

If the weapon clips into the head or torso:

- raise the min muzzle distance

## Layering

Suggested render order for a right-facing character:

1. lower body
2. back upper arm
3. back forearm
4. torso
5. head
6. weapon
7. front upper arm
8. front forearm

When facing flips, keep the same semantic layering:

- `front_*` stays nearer to camera
- `back_*` stays farther from camera

Do not reinterpret front/back as literal left/right assets.

## Clothing Variants

Different outfits should reuse the same anchor contract.

Good approach:

- each outfit exports the same parts
- each part keeps compatible pivots
- only art changes, not gameplay rig logic

Example variant ids:

- `man1_jacket`
- `man1_armor_light`
- `man1_armor_heavy`
- `man1_hazmat`

## Weapon Interaction

Weapon art should define:

- held sprite
- rear grip
- front grip
- muzzle

Character art should not bake weapon-holding posture into the arm sprite itself.

## First Playable Milestone

To unblock rendering quickly, the minimum shippable set is:

- lower-body walk strip
- head
- torso
- front upper arm
- front forearm
- back upper arm
- back forearm

Everything else can be layered on later.
