# UI Layout Spec

This document proposes the first stable layout format for scene-driven UI such as the workbench, loadout, and future inventory screens.

## Design Goals

- Keep control skins and control placement separate.
- Author screen layouts without recompiling C++ for every pixel tweak.
- Stay game-UI friendly: fixed logical pixels, anchors, stacking, and named regions matter more than full CSS flexibility.

## Why Not CSS

CSS solves browser problems. Our game UI has different priorities:

- fixed logical canvas coordinates
- explicit layering order
- pixel-aligned anchors
- simple adaptive behavior rather than arbitrary flow layout
- metadata that maps cleanly to C++ structs

We want something lighter than CSS and more structured than hardcoded rectangles.

## Recommended Structure

Store one JSON file per screen under:

- `assets/ui/layouts/`

Example:

```json
{
  "canvas": { "w": 640, "h": 480 },
  "elements": [
    {
      "id": "weapon_list_panel",
      "type": "panel",
      "skin": "panel_square_bronze",
      "anchor": "top_left",
      "rect": { "x": 12, "y": 56, "w": 120, "h": 308 },
      "title": "武器列表",
      "z": 20
    }
  ]
}
```

## Core Fields

### `canvas`

Logical design resolution for the layout file.

### `elements`

Flat ordered list of UI elements. Each element has:

- `id`
- `type`
- `anchor`
- `rect`
- `z`

Optional per-type fields:

- `skin`
- `title`
- `text`
- `icon`
- `orientation`
- `item_template`
- `children`

## Anchor Model

Supported anchors should stay simple:

- `top_left`
- `top_right`
- `bottom_left`
- `bottom_right`
- `center`
- `top_center`
- `bottom_center`
- `left_center`
- `right_center`

Anchors resolve the element's logical rect relative to the screen or parent container.

## Suggested Element Types

- `panel`
- `card`
- `button`
- `slot`
- `scrollbar`
- `label`
- `icon`
- `progress_bar`
- `list`

## Screen Families

The roadmap implies several recurring UI families. We should treat them as first-class screen categories instead of ad hoc pages:

- `title_screen`
- `workbench_screen`
- `inventory_screen`
- `build_mode_overlay`
- `quick_action_overlay`
- `detail_panel_screen`
- `radio_trade_screen`
- `npc_task_screen`
- `medical_screen`
- `night_defense_overlay`

This matters because the same controls will be reused across multiple contexts:

- `panel` for framed areas
- `card` for weapon rows, NPC rows, event rows
- `slot` for attachment sockets, inventory cells, build categories
- `scrollbar` for long lists
- `button` for actions and confirmations

## Interaction Layers

The roadmap describes a three-layer interaction model. The UI system should support these layers explicitly:

### Layer 1: Hover Info

Lightweight tooltip-style information:

- current object name
- power/noise/light summary
- quick risk hints

### Layer 2: Quick Action Panel

Fast context actions:

- repair
- toggle
- harvest
- assign
- build
- upgrade

### Layer 3: Detail Management Panel

Full management views:

- complete stats
- requirements
- task queue
- upgrades
- room purpose
- NPC assignment

This layered model helps us avoid overloading one screen with every interaction at once.

## Suggested Composition Rules

### `panel`

Static framed container with optional title.

### `card`

Clickable item tile or list row. Usually uses a button-like skin plus richer content.

### `slot`

Item socket or inventory cell. Can contain icon, quantity, rarity mark, lock state.

### `scrollbar`

Track plus thumb. Orientation is `vertical` or `horizontal`.

### `list`

Scrollable region with:

- viewport rect
- item spacing
- repeated item template
- scrollbar binding

This is the right abstraction for the weapon list panel rather than manually placing every row in code.

## Data Binding Direction

The layout file should describe:

- where elements live
- what control type they are
- which named style they use
- how they anchor and stack

Gameplay code should still own:

- which rows are populated
- which slot is selected
- which actions are enabled
- which stats or resources are displayed

That gives us a clean split:

- `ControlStyle` defines appearance
- `layout JSON` defines placement
- `screen/view-model code` defines live data and interaction

## Recommended First Step

For the first workbench screen:

- keep high-level screen composition in JSON
- keep low-level item content binding in C++

That means the layout file owns:

- panel positions
- title banner position
- list viewport rect
- stats panel rect
- upgrade panel rect
- attachment strip rect

But C++ still decides:

- which weapons populate the list
- which stats text is shown
- which attachment slots are locked

This is a good midpoint between flexibility and implementation effort.
