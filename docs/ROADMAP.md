# ZombieGame Modernization Roadmap

This project started as a Win32/GDI C++ practice game. The modernization goal is to preserve the pixel-art survival-shooter feel while making the codebase easier to build, port, and extend.

## Phase 0: Keep The Original Playable

- Build from VS Code or a terminal with CMake.
- Keep the existing Win32/GDI renderer intact as the reference implementation.
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
