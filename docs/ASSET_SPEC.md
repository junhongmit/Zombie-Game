# Asset Pipeline Spec

This document defines the first stable storage convention for the SDL3 branch.

## Goals

- Keep the original `image/` tree intact for baseline Win32 comparison.
- Move the SDL3 runtime toward metadata-driven assets.
- Let new gameplay, enemies, UI skins, and maps be authored without editing C++ draw code.

## Directory Layout

- `image/`
  Legacy source assets used by the archived Win32 baseline and as migration input.
- `assets/characters`
  Animation metadata and future packed atlases for hero, zombies, and NPCs.
- `assets/effects`
  Explosion, smoke, grenade, and other effect atlases plus metadata.
- `assets/ui`
  Packed control skins, icon sheets, and UI metadata.
- `assets/collision`
  Future map collision layers, trigger layers, and navigation masks.
- `tools/`
  Build scripts that convert loose source frames into atlases and metadata.

## Sprite Sheet Metadata

Current `.sheet` format is intentionally simple:

```ini
frame_width=18
frame_height=33
stride_x=17
frame_count=24
```

This is enough for strips where all frames share one row and one pivot convention.

Planned extensions:

- `pivot_x`
- `pivot_y`
- `fps`
- `frame_duration`
- `hitbox`
- `event markers`
- multi-row atlas support

## UI Skin Metadata

UI controls need more than animation frames. They also need scalable borders and content padding.

Recommended storage format: JSON sidecar per packed UI sheet.

Example:

```json
{
  "sheet": "ui_controls.png",
  "controls": {
    "button_primary": {
      "states": {
        "normal": { "x": 0, "y": 0, "w": 64, "h": 24 },
        "hover":  { "x": 64, "y": 0, "w": 64, "h": 24 },
        "pressed": { "x": 128, "y": 0, "w": 64, "h": 24 },
        "disabled": { "x": 192, "y": 0, "w": 64, "h": 24 }
      },
      "slice": { "left": 6, "right": 6, "top": 6, "bottom": 6 },
      "content_padding": { "left": 10, "right": 10, "top": 6, "bottom": 6 },
      "min_size": { "w": 24, "h": 24 }
    }
  }
}
```

### `slice`

This is a 9-slice definition:

- corners are fixed
- edge strips stretch in one axis
- center stretches in both axes

That solves the exact problem you described: the program knows which parts may scale and which must stay pixel-perfect.

### `content_padding`

Defines where text or icons may be placed inside the control after scaling.

### `min_size`

Prevents scaling below the point where fixed borders overlap.

## Collision And Map Data

Short term:

- keep using legacy `building form.png` for the SDL3 prototype
- preserve color semantics for solid and stair triggers

Long term:

- replace color-coded collision authoring with exported data layers
- separate:
  - visual tiles
  - solid collision
  - triggers
  - navigation waypoints
  - loot spawn markers
  - enemy spawn markers

Recommended future format:

- Tiled JSON or a small custom JSON bundle

## Deprecation Policy

- Do not delete legacy `image/` assets yet.
- The Win32 baseline remains the behavior reference.
- New SDL3-facing assets should be added under `assets/...`.
- Once a resource category is fully migrated and verified, we can move the old source assets into an `archived/legacy_assets/` tree instead of deleting them.
