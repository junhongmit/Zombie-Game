# ZombieGame Modernization Roadmap

This project started as a Win32/GDI C++ practice game. The modernization goal is to preserve the pixel-art survival-shooter feel while making the codebase easier to build, port, and extend.

## Phase 0: Preserve The Baseline

- Build from VS Code or a terminal with CMake.
- Keep the archived Win32/GDI baseline available as a behavioral reference when needed.
- Avoid gameplay rewrites until the current behavior is easy to run and compare.

## Phase 1: Extract Game State

- Move shared structs and constants into a small `src/game` layer.
- Separate update logic from `WM_TIMER` and Win32 messages.
- Replace timer-driven movement with a fixed or semi-fixed timestep.
- Keep rendering as an adapter around the existing state.

## Phase 2: Replace The Renderer

Recommended first target: SDL3 or raylib.

- Load images into GPU textures instead of HDC-backed bitmaps.
- Replace `BitBlt`, `TransparentBlt`, `AlphaBlend`, and `PlgBlt` calls with sprite draw calls.
- Keep a 640x480 logical resolution and scale to the window.
- Batch static background layers and draw dynamic actors separately.
- Add full-screen impact feedback such as screen shake for gunshots, explosions, and heavy zombie hits.
- Move sprite-sheet metadata out of ad hoc render code and into reusable animation descriptors.

## Phase 3: Rebuild Navigation

- Replace per-zombie pixel BFS with a building navigation graph.
- Represent floors, stairs, doors, windows, and drop points as nodes and edges.
- Recompute target paths only when player/girlfriend target nodes change.
- Give zombies different target preferences and movement abilities.

## Phase 4: Grow The Game

- Add defense decisions: barricades, repairs, traps, lights, doors.
- Add resource pressure: ammo, medkits, power, noise, limited wave preparation time.
- Add mission nights: survive, rescue, repair, relocate, defend multiple rooms.
- Make weapons tactically distinct rather than only higher DPS.

## Backlog: Systems To Revisit

- Introduce a shared actor/combat base layer so `Player` and `Zombie` can eventually share damage, impulse, status, and explosion-response code.
- Replace color-coded collision authoring with a more scalable level format for future locations such as school, supermarket, and hospital maps.
- Move loose frame-by-frame animation assets toward sprite sheets or atlases with metadata for frame rects, pivots, timing, and hit regions.
- Add a proper asset build step that can package atlases and metadata into a shipping-friendly bundle format.
- Migrate HUD, menu, and shop text from custom debug/pixel drawing to `SDL3_ttf`.
- Add a reusable UI skin system with 9-slice metadata, content padding, per-state frames, and minimum-size rules so buttons, tabs, sliders, and panels can scale cleanly.
- Add semantic hit regions for enemies beyond simple head/body height checks once zombie variety expands.
- Explore a future art pass using GPT Images for new props, enemy variants, and environment set dressing once the runtime asset pipeline is in place.
