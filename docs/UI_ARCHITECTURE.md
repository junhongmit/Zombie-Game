# UI Architecture

This document defines the working UI architecture for the SDL3 rewrite. The goal is to keep the game UI scalable as we add more screens, more lists, more interaction modes, and more content-heavy management views.

## Principles

- Prefer reusable controls over screen-local drawing code.
- Separate appearance, layout, and runtime data binding.
- Use a stable logical UI resolution and let presentation scaling happen at the edge.
- Make parent/child relationships explicit so moving one container does not explode neighboring controls.
- Keep screen code focused on state and interaction, not pixel math.

## Layers

### 1. Appearance

Appearance lives in skins and assets:

- `ControlStyle`
- UI sprites / 9-slice skins
- icon textures
- font choices

Appearance answers:

- how a control looks
- how its visual states differ
- where built-in content regions live

### 2. Layout

Layout lives in JSON and container trees:

- screen layout JSON under `assets/ui/layouts/`
- normalized screen rects
- container child rects
- named slots such as `header`, `body`, `icon`, `title`

Layout answers:

- where a control lives
- how big it is
- which children it owns
- which slot each child targets

### 3. Binding / Interaction

Binding lives in screen code:

- live strings
- resource counts
- list entries
- selection state
- hover / press / drag behavior

Binding answers:

- what content is shown right now
- which control is selected, armed, locked, or disabled
- what happens on click, drag, scroll, or hotkey

## Core Types

### Leaf items

Leaf items do not own children.

- `TextItem`
- `IconItem`
- `BoxItem`

Use leaf items for:

- labels
- numeric readouts
- icons
- simple bars / fills / framed overlays

### Containers

Containers own children and define slot semantics.

- `Canvas`
- `Panel`
- `Button`
- `Card`
- `ListView`

Use containers for:

- framed panels
- clickable actions
- item rows
- screen sections
- scrollable repeated content

## Slot Semantics

Named slots are the contract between a control and its children.

### `Panel`

- `header`
- `title`
- `header_action`
- `body`
- `footer`

### `Button`

- `icon`
- `icon_center`
- `label`
- `title`
- `text`
- `label_after_icon`
- `title_after_icon`
- `badge`
- `content`

### `Card`

- `icon`
- `icon_center`
- `title`
- `subtitle`
- `meta`
- `badge`
- `text_block`
- `content`

If a new control needs screen-specific geometry, add named slots instead of pushing more hardcoded local offsets into screen render functions.

## Control Responsibilities

### `Panel`

Owns framed UI sections with optional title and header/footer semantics.

### `Button`

Owns clickable action surfaces. Button interaction rules should be shared everywhere:

- press arms only when pointer starts inside
- release triggers only when pointer is still inside
- drag-out cancels pressed visuals

### `Card`

Owns rich item presentation such as weapon rows, inventory entries, shop entries, and survivor cards.

### `ListView`

Owns viewport, scroll extent, and scrollbar binding. Repeated row content should be driven through templates or child factories rather than screen-local pixel loops long-term.

## Current Coding Rule

When building or refactoring UI:

1. Put screen placement in layout JSON when the rect is screen-owned.
2. Put child composition inside a container when the child belongs to that control.
3. Use `TextItem`, `IconItem`, and `BoxItem` for leaf content instead of direct `render_text(...)` or raw SDL fill calls where practical.
4. Let screens bind data into controls; do not let screens micromanage every text baseline if the control can own it.

## What Still Counts As Legacy

These patterns are now considered transitional and should be reduced over time:

- screen functions that manually render long runs of labels with fixed offsets
- screen code that draws raw rectangles for reusable UI chrome
- list rows assembled entirely by local pixel math instead of card/list abstractions
- hover/pressed logic copied per screen instead of going through shared controls

## Recommended Next Targets

The biggest remaining immediate-mode hotspots are:

- `HudRenderer.cpp`
- workbench stats/effects/upgrade body content
- inventory resource rows and weight section internals
- repeated list-row factories that still hand-place text and icons

These are the next modernization steps before or alongside the black market screen.

