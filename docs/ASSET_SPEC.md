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

## Scene Manifest

Large scene textures should be routed through:

- `assets/scenes/scenes.json`

Current supported scene keys:

- `sky`
- `backcity1`
- `backcity2`
- `backcity3`
- `building`
- `bench`
- `market`
- `notebook`

Example:

```json
{
  "scenes": {
    "market": {
      "path": "market2.png"
    },
    "notebook": {
      "path": "notebook2.png"
    }
  }
}
```

This allows material replacement of major backgrounds and foreground plates without touching C++.

## Weapon Preview Naming

Primary weapon metadata should now live in:

- `assets/weapons/weapons.json`

Legacy `assets/weapons/weapon.ini` can remain as a compatibility fallback during migration, but new weapon content should be authored in JSON first.

For SDL3 weapon assets, use one normalized weapon id in `snake_case` and keep each weapon in its own folder.

Recommended convention:

- weapon folder:
  - `assets/weapons/<weapon_id>/`
- preview sprite:
  - `assets/weapons/<weapon_id>/preview.png`
- preview depth map:
  - `assets/weapons/<weapon_id>/preview_depth.png`
- inventory/workbench icon:
  - `assets/weapons/<weapon_id>/icon.png`
- held/in-game sprite:
  - `assets/weapons/<weapon_id>/hold.png`
- weapon sounds:
  - `assets/weapons/<weapon_id>/sounds/<sound_name>.wav`

Examples:

- `assets/weapons/glock/preview.png`
- `assets/weapons/glock/preview_depth.png`
- `assets/weapons/glock/icon.png`
- `assets/weapons/glock/hold.png`
- `assets/weapons/glock/sounds/glock18-2.wav`
- `assets/weapons/desert_eagle/preview.png`
- `assets/weapons/desert_eagle/preview_depth.png`
- `assets/weapons/desert_eagle/icon.png`
- `assets/weapons/desert_eagle/hold.png`

Notes:

- New assets should avoid spaces and mixed casing.
- Treat the `weapon_id` folder as the canonical asset bundle for preview art, depth art, icons, held sprites, sounds, metadata, and future localization.
- Legacy flat names from `weapon.ini` such as `hold desert eagle.png` should be migrated into weapon folders when touched.

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

## Character Rig Parts

For the new upper-body aiming rig, character art should move away from "weapon + arm baked together" sprites and toward authored modular body parts.

Recommended storage:

- sprite sheet and metadata:
  - `assets/characters/<character_id>_rig.png`
  - `assets/characters/<character_id>_rig.json`

Recommended first-pass parts:

- `head`
- `torso`
- `front_upper_arm`
- `front_forearm`
- `back_upper_arm`
- `back_forearm`

Optional later parts:

- `front_hand`
- `back_hand`
- `front_shoulder_pad`
- `back_shoulder_pad`
- `pelvis`
- `vest_overlay`
- `armor_overlay`

Recommended principle:

- lower body stays as a separate animated strip / atlas
- upper body is pose-driven by the aiming rig
- clothing variants swap upper-body sheets and keep the same anchor contract

See:

- `assets/characters/CHARACTER_RIG_SPEC.md`
- `assets/characters/man1_rig.example.json`

## UI Skin Metadata

UI controls need more than animation frames. They also need scalable borders, explicit fill behavior, optional decorative layers, and content padding.

Recommended storage format: JSON sidecar per packed UI sheet.

Example:

```json
{
  "sheet": "ui_controls.png",
  "controls": {
    "button_primary": {
      "states": {
        "normal": {
          "frame": { "x": 0, "y": 0, "w": 64, "h": 24 },
          "border": {
            "slice": { "left": 6, "right": 6, "top": 6, "bottom": 6 }
          },
          "fill": {
            "mode": "stretch",
            "rect": { "x": 6, "y": 6, "w": 52, "h": 12 }
          }
        }
      },
      "content_padding": { "left": 10, "right": 10, "top": 6, "bottom": 6 },
      "min_size": { "w": 24, "h": 24 }
    }
  }
}
```

### `style_scale`

Optional uniform scale hint for high-resolution UI source art.

Example:

```json
"style_scale": 0.5
```

This does not replace the destination rect chosen by layout code. It is a metadata hint that higher-level controls can use when they want a sane default display size for oversized source art such as high-resolution cards.

### `border.slice`

This is a 9-slice definition:

- corners are fixed
- edge strips stretch in one axis
- center stretches in both axes

That tells the renderer how to scale the border shell while keeping corners and trim pixel-perfect.

### `fill`

The center of many controls should not be treated as an accidental byproduct of 9-slice. It needs its own rule.

Recommended fields:

- `mode`
  - `stretch`: scale one source patch to the interior
  - `tile`: repeat a small texture across the interior
  - `fixed`: draw one unscaled patch centered or anchored inside the interior
- `rect`
  - the source rectangle inside the sprite sheet used as the fill source
- `tile`
  - optional tile size metadata for tiled fills

This matters for:

- buttons with leather or metal centers
- tabs with a distinct plate interior
- inventory slots with dark recessed fills
- windows and panels with parchment, cloth, or gridded backgrounds

In other words: 9-slice handles the shell; `fill` handles the inside.

### `decor`

Optional extra layers that are neither border nor fill:

- corner badges
- tab notches
- clipped top ornaments
- fixed screws, emblems, or ribbons

These should support anchor-based placement such as:

- `top_left`
- `top_right`
- `bottom_left`
- `bottom_right`
- `center`

### `content_padding`

Defines where text or icons may be placed inside the control after scaling.

### `regions`

Optional named sub-rectangles inside a control state. These are not decorative; they are semantic content slots used by higher-level controls.

Recommended examples:

- `icon`
- `title`
- `subtitle`
- `meta`
- `badge`
- `value`
- `track`
- `thumb`
- `grip`

Why this matters:

- a `card` is not just a frame; it needs consistent places for icon, name, subtype, and optional lock/star badges
- a `scrollbar` is not just a long strip; the renderer needs to know where the thumb art lives and where the grip decoration sits

These regions should be authored in frame-local coordinates unless a control explicitly defines a separate decorated source rectangle.

### Separate grip / decor textures

If a scrollbar thumb or other control uses a small ornament that should not be stretched with the center fill, prefer a separate decor definition:

```json
{
  "scrollbar_vertical_thumb": {
    "states": {
      "normal": {
        "frame": { "x": 40, "y": 100, "w": 41, "h": 81 },
        "border": {
          "slice": { "left": 9, "right": 9, "top": 12, "bottom": 12 }
        },
        "fill": {
          "mode": "stretch",
          "rect": { "x": 49, "y": 114, "w": 23, "h": 55 }
        }
      }
    },
    "grip": {
      "rect": { "x": 220, "y": 40, "w": 12, "h": 20 },
      "anchor": "center",
      "offset": { "x": 0, "y": 0 }
    }
  }
}
```

Use this when:

- the grip art is packed outside the thumb frame
- the grip should stay fixed size
- the grip should not be affected by 9-slice center stretching

Supported anchors:

- `center`
- `top`
- `bottom`
- `left`
- `right`
- `top_left`
- `top_right`
- `bottom_left`
- `bottom_right`

`offset` is applied in authored control-space pixels and scales with the control on screen.

### `min_size`

Prevents scaling below the point where fixed borders overlap.

### `max_size`

Optional upper bound for controls that should not become visually awkward when the layout gives them too much room.

This is especially useful for:

- scrollbar thumbs
- compact slots
- icon-like controls

Example:

```json
"max_size": { "w": 41, "h": 96 }
```

This does not force the control to always render at that size. It simply caps auto-grown dimensions when a higher-level widget computes a dynamic size.

## Recommended UI Skin Model

Treat each control state as a small layered composition:

1. `frame`
   - the full source rectangle for that control state
2. `border`
   - usually 9-slice
3. `fill`
   - stretch, tile, or fixed interior
4. `decor`
   - optional overlays anchored to corners or edges
5. `content_padding`
   - safe area for labels, icons, counters, or item art
6. `regions`
   - optional semantic placement slots for structured controls

This model scales well across:

- buttons
- tabs
- item slots
- modal panels
- windows
- progress bars
- toggles

The important shift is that we should not think of a control as "just a border". We should think of it as a composed skin with explicit rules for shell, interior, and ornament.

## Card Skin Convention

Cards are richer than buttons. They usually contain:

- an item or weapon icon
- a title
- a subtitle or type label
- optional meta text such as quantity or ammo
- optional status badge such as lock, rarity, or favorite

Recommended metadata shape:

```json
{
  "card_weapon_row": {
    "states": {
      "normal": { "...": "same frame/border/fill structure as other controls" },
      "hover": { "...": "..." },
      "pressed": { "...": "..." },
      "disabled": { "...": "..." }
    },
    "content_padding": { "left": 8, "right": 8, "top": 8, "bottom": 8 },
    "regions": {
      "icon": { "x": 8, "y": 8, "w": 28, "h": 28 },
      "title": { "x": 42, "y": 7, "w": 84, "h": 12 },
      "subtitle": { "x": 42, "y": 21, "w": 84, "h": 10 },
      "badge": { "x": 128, "y": 6, "w": 10, "h": 10 },
      "meta": { "x": 42, "y": 32, "w": 84, "h": 10 }
    },
    "min_size": { "w": 72, "h": 40 }
  }
}
```

### Card art guidance

- Keep icon area visually quieter than the text area.
- Make selected state stronger than hover.
- Keep the subtitle region readable on dark fills.
- Reserve a small badge corner even if the first implementation does not use it yet.

For now, selected rows can reuse the `pressed` state. If we later need a separate selected-only state, we can add that at the control-system level without changing the basic card structure.

## Scrollbar Skin Convention

Scrollbars should be modeled as compound controls:

- `scrollbar_vertical_track`
- `scrollbar_vertical_fill`
- `scrollbar_vertical_thumb`
- `scrollbar_horizontal_track`
- `scrollbar_horizontal_fill`
- `scrollbar_horizontal_thumb`

This is better than trying to store track and thumb in one monolithic control definition because:

- track and thumb stretch differently
- the filled/consumed region usually has a different value glow or tint than the unfilled track
- thumb often needs its own hover/pressed feedback
- horizontal and vertical versions may not share the same cap art

Recommended track/thumb regions:

- `channel`
- `thumb_bounds`
- `fill_track`

The fill control represents the lit/consumed/selected portion of the rail. It is rendered inside the track bounds using the current normalized scroll progress.

### Scrollbar semantics

- `channel`
  - the narrow active slot where the rail visually lives
  - useful when the thumb is wider than the rail
- `thumb_bounds`
  - the full area the thumb is allowed to travel within
  - lets the thumb overhang the rail while still clamping cleanly
- `fill_track`
  - the exact strip used for the "already scrolled" or "progress" region
  - this can be centered, upper, or lower relative to the rail depending on the art

Recommended metadata shape:

```json
{
  "scrollbar_vertical_thumb": {
    "states": {
      "normal": {
        "frame": { "x": 0, "y": 0, "w": 18, "h": 40 },
        "border": {
          "slice": { "left": 5, "right": 5, "top": 8, "bottom": 8 }
        },
        "fill": {
          "mode": "stretch",
          "rect": { "x": 5, "y": 8, "w": 8, "h": 24 }
        }
      }
    },
    "regions": {
      "grip": { "x": 6, "y": 15, "w": 6, "h": 10 }
    },
    "min_size": { "w": 18, "h": 24 }
  }
}
```

### Scrollbar art guidance

- Track should be visually quieter than thumb.
- Filled track should clearly separate completed/active area from remaining/inactive area.
- Thumb must still read well at minimum size.
- Vertical and horizontal thumbs should have clean center strips for stretching.
- Keep grip decoration optional; it may disappear at tiny sizes.

## Recommended First Production Skins

For the workbench, the highest-value first batch is:

- `card_weapon_row`
- `card_attachment`
- `slot_attachment`
- `slot_part`
- `scrollbar_vertical_track`
- `scrollbar_vertical_fill`
- `scrollbar_vertical_thumb`
- `scrollbar_horizontal_track`
- `scrollbar_horizontal_fill`
- `scrollbar_horizontal_thumb`

That set is enough to build:

- weapon list
- attachment strip
- parts grid
- scrollable inventory/list panels

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
