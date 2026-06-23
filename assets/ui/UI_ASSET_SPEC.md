# UI Asset Spec

This file describes the intended metadata conventions for the game's reusable UI skin system.

It is meant for authoring new assets, especially:
- panels
- header rows
- dividers
- icon buttons such as close `X`
- cards and tooltip panels

Some fields below are already supported by runtime code, while others are part of the forward-looking spec so art can be prepared before all behavior is wired in.

## Directory Structure

Recommended folders:

```text
assets/ui/
  buttons/
  cards/
  dividers/
  headers/
  icons/
  layouts/
  panels/
  progress/
  scrolls/
```

## Base Control Format

Each metadata file follows:

```json
{
  "sheet": "texture.png",
  "controls": {
    "control_name": { }
  }
}
```

Each control should define:

```json
{
  "style_scale": 1.0,
  "states": {
    "normal": { },
    "hover": { },
    "pressed": { },
    "disabled": { }
  },
  "content_padding": { "left": 0, "right": 0, "top": 0, "bottom": 0 },
  "regions": { },
  "min_size": { "w": 0, "h": 0 },
  "max_size": { "w": 0, "h": 0 }
}
```

## State Format

Each visual state uses:

```json
{
  "frame": { "x": 0, "y": 0, "w": 100, "h": 50 },
  "border": {
    "slice": { "left": 8, "right": 8, "top": 8, "bottom": 8 }
  },
  "fill": {
    "mode": "stretch",
    "rect": { "x": 8, "y": 8, "w": 84, "h": 34 }
  }
}
```

Fill modes:
- `stretch`
- `tile`
- `fixed`

## Canonical Region Space

All regions should be authored against the **normal** state frame.

That means `title`, `icon`, `meta`, `header_action`, `divider_top`, etc. should all line up using the normal-state coordinate system, even if hover/pressed frames differ slightly in size.

## Common Regions

Currently used or reserved region keys:

```json
{
  "icon": { "x": 0, "y": 0, "w": 0, "h": 0 },
  "title": { "x": 0, "y": 0, "w": 0, "h": 0 },
  "subtitle": { "x": 0, "y": 0, "w": 0, "h": 0 },
  "badge": { "x": 0, "y": 0, "w": 0, "h": 0 },
  "meta": { "x": 0, "y": 0, "w": 0, "h": 0 },
  "header_row": { "x": 0, "y": 0, "w": 0, "h": 0 },
  "header_title": { "x": 0, "y": 0, "w": 0, "h": 0 },
  "header_action": { "x": 0, "y": 0, "w": 0, "h": 0 },
  "divider_top": { "x": 0, "y": 0, "w": 0, "h": 0 },
  "body": { "x": 0, "y": 0, "w": 0, "h": 0 },
  "track": { "x": 0, "y": 0, "w": 0, "h": 0 },
  "fill_track": { "x": 0, "y": 0, "w": 0, "h": 0 },
  "channel": { "x": 0, "y": 0, "w": 0, "h": 0 },
  "thumb_bounds": { "x": 0, "y": 0, "w": 0, "h": 0 },
  "grip": { "x": 0, "y": 0, "w": 0, "h": 0 }
}
```

## Panel Spec

Panels should be designed as:
- an outer frame
- a header row
- a divider between header and body
- a content body area

Recommended panel regions:

```json
{
  "regions": {
    "title": { "x": 14, "y": 10, "w": 68, "h": 16 },
    "header_row": { "x": 10, "y": 8, "w": 76, "h": 18 },
    "header_title": { "x": 14, "y": 10, "w": 54, "h": 16 },
    "header_action": { "x": 68, "y": 8, "w": 18, "h": 18 },
    "divider_top": { "x": 10, "y": 28, "w": 76, "h": 2 },
    "body": { "x": 14, "y": 34, "w": 68, "h": 48 }
  }
}
```

Intended semantics:
- `header_row`: overall top strip
- `header_title`: title text anchor area
- `header_action`: close or utility button slot
- `divider_top`: decorative separator line
- `body`: preferred content area for child controls

## Divider Spec

Dividers should be their own lightweight control family.

Suggested names:
- `divider_horizontal_bronze`
- `divider_vertical_bronze`

Horizontal divider example:

```json
{
  "states": {
    "normal": {
      "frame": { "x": 224, "y": 324, "w": 48, "h": 4 },
      "border": {
        "slice": { "left": 2, "right": 2, "top": 1, "bottom": 1 }
      },
      "fill": {
        "mode": "stretch",
        "rect": { "x": 226, "y": 325, "w": 44, "h": 2 }
      }
    }
  }
}
```

## Header Row Spec

If a panel family uses a dedicated header strip texture, treat it as its own control:

Suggested names:
- `header_row_bronze`
- `header_row_dark`

Typical regions:

```json
{
  "regions": {
    "title": { "x": 16, "y": 8, "w": 140, "h": 20 },
    "icon": { "x": 160, "y": 6, "w": 20, "h": 20 },
    "badge": { "x": 132, "y": 8, "w": 24, "h": 16 }
  }
}
```

## Icon Button Spec

Use a dedicated icon button control for close/settings/filter buttons.

Suggested names:
- `icon_button_close`
- `icon_button_small_bronze`

Recommended regions:

```json
{
  "regions": {
    "icon": { "x": 4, "y": 4, "w": 16, "h": 16 }
  },
  "min_size": { "w": 18, "h": 18 },
  "max_size": { "w": 32, "h": 32 }
}
```

## Tooltip Panel Spec

Tooltip panels should be authored with:
- compact frame
- small body padding
- divider between title and body

Recommended conceptual layout:

```json
{
  "regions": {
    "title": { "x": 14, "y": 8, "w": 120, "h": 20 },
    "divider_top": { "x": 12, "y": 32, "w": 140, "h": 2 },
    "body": { "x": 14, "y": 38, "w": 136, "h": 72 }
  },
  "content_padding": { "left": 12, "right": 12, "top": 10, "bottom": 10 }
}
```

## Text Role Guidance

Keep these roles in mind when designing control interiors:
- `section_font`: major section headers
- `title_font`: card and button labels
- `body_font`: normal UI copy
- `small_font`: secondary labels
- `number_font`: values, quantities, weights
- `key_font`: keycaps and shortcut glyphs

Practical implication:
- leave stable horizontal lanes for titles
- reserve vertical bands for title / subtitle / meta on cards
- do not rely on decorative fill itself to imply text position

## Recommended Naming Pattern

```text
<category>_<shape>_<theme>_<variant>
```

Examples:
- `panel_square_bronze`
- `header_row_bronze`
- `icon_button_close`
- `divider_horizontal_bronze`
- `card_weapon_row`
- `card_backpack_slot`

## Current Priority Assets

The next most useful UI assets to add are:

1. richer panel skin with explicit header/divider treatment
2. dedicated `icon_button_close`
3. horizontal divider
4. vertical divider
5. dedicated tooltip panel skin

## Compatibility Note

Not every region in this spec is fully consumed by runtime code yet.

That is intentional:
- art can be produced ahead of code
- metadata remains stable
- runtime can progressively adopt `header_row`, `header_action`, `divider_top`, and `body` without forcing reauthoring
