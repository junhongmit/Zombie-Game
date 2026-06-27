# Weapon Hold Metadata Spec

Use a small sibling JSON next to a held weapon sprite when the grip layout should be
authored per-asset.

Example:

```text
assets/weapons/glock/
  hold_glock.png
  hold_glock.json
```

## Fields

- `mirrored_pair_sprite`
  - `true` when the sprite stores two half-frames side by side
  - `false` when the sprite is a single right-facing image that should simply flip at runtime
- `hold_scale`
  world-size multiplier for high-resolution held sprites
- `rear_wrist`
  wrist attachment point for the firing hand
- `front_wrist`
  wrist attachment point for the support hand
- `muzzle`
  muzzle point used for bullet spawn and aim debugging

All points are authored in sprite pixels in the weapon's right-facing local frame.

## Example

```json
{
  "mirrored_pair_sprite": false,
  "hold_scale": 0.12,
  "rear_wrist": { "x": 10, "y": 67 },
  "front_wrist": { "x": 23, "y": 71 },
  "muzzle": { "x": 151, "y": 33 }
}
```

## Tuning Notes

- move `rear_wrist` to change where the main forearm plugs into the held sprite
- move `front_wrist` to change where the support forearm lands
- move `muzzle` only when the bullet/debug line does not leave from the true barrel end
- lower `hold_scale` when a beautifully detailed source sprite is simply too large for gameplay

Important:

- points are authored in sprite pixels with `+Y down`
- runtime code converts them into rig-local offsets with `+Y up`
- runtime code also multiplies those offsets by `hold_scale`

For pistols, `front_wrist` often sits close to `rear_wrist`.
For rifles and shotguns, `front_wrist` usually lives much farther forward under the handguard.
