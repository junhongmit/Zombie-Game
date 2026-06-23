# ZombieGame

A pixel-art side-scrolling zombie survival shooter now maintained on an SDL3-based mainline.

Active development now happens in `src/` and `assets/`. Legacy Win32/GDI experiments can still live locally for reference, but they are no longer part of the main tracked branch.

## Recommended Windows Setup

Develop this project as a native Windows program.

Recommended stack:

- VS Code
- MSYS2 UCRT64
- GCC / GDB
- CMake
- Ninja

## 1. Install VS Code

Install VS Code from:

https://code.visualstudio.com/

Recommended extensions:

- C/C++ by Microsoft
- CMake Tools by Microsoft

Optional:

- Git for Windows: https://git-scm.com/download/win

## 2. Install MSYS2

Install MSYS2 from:

https://www.msys2.org/

Use the default install path:

```text
C:\msys64
```

Open **MSYS2 UCRT64** and update:

```bash
pacman -Syu
```

If prompted, reopen **MSYS2 UCRT64** and run it again.

## 3. Install The Toolchain

In **MSYS2 UCRT64**, run:

```bash
pacman -S --needed mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-gdb mingw-w64-ucrt-x86_64-cmake mingw-w64-ucrt-x86_64-ninja mingw-w64-ucrt-x86_64-pkgconf git
```

Install SDL dependencies:

```bash
pacman -S --needed mingw-w64-ucrt-x86_64-sdl3 mingw-w64-ucrt-x86_64-sdl3-image mingw-w64-ucrt-x86_64-sdl3-ttf
```

For optional Python asset scripts:

```powershell
python -m pip install pillow
```

## 4. Add Tools To Windows PATH

Add this folder to your Windows user PATH:

```text
C:\msys64\ucrt64\bin
```

Then restart PowerShell and VS Code.

Verify:

```powershell
gcc --version
g++ --version
gdb --version
cmake --version
ninja --version
```

## 5. Open The Project

Open:

```text
D:\Code\game
```

Then run:

```text
Tasks: Run Build Task
```

## Build From Terminal

From PowerShell in `D:\Code\game`:

```powershell
cmake -S . -B build -G Ninja -D CMAKE_BUILD_TYPE=Debug
cmake --build build
.\build\bin\zombie_game.exe
```

Release build:

```powershell
cmake -S . -B build-release -G Ninja -D CMAKE_BUILD_TYPE=Release
cmake --build build-release
.\build-release\bin\zombie_game.exe
```

## Debug In VS Code

Use:

```text
Run and Debug -> Debug zombie_game
```

That launches:

```text
build\bin\zombie_game.exe
```

with the workspace root as `cwd`, so the runtime can resolve `assets/`, `music/`, and `sound/`.

## Resource Layout

Active SDL3 resources live under:

- `assets/backgrounds`
- `assets/characters`
- `assets/collision`
- `assets/effects`
- `assets/ui`
- `assets/weapons`

See [ASSET_SPEC.md](D:/Code/game/docs/ASSET_SPEC.md) for the current storage conventions.

## Asset Packing

If you want to pack loose frames into a strip:

```powershell
python tools/pack_spritesheet.py `
  --input-dir some/source/folder `
  --pattern "boom1_*.bmp" `
  --output-image assets/effects/boom1_strip.png `
  --output-meta assets/effects/boom1_strip.sheet
```

## Weapon Relight Prototype

For the workbench weapon-lighting prototype:

```powershell
python tools/weapon_relight.py `
  --albedo assets/weapons/glock.png `
  --height path/to/glock_depth.png `
  --output tmp/glock_lit.png `
  --normal-output tmp/glock_normal.png `
  --light-output tmp/glock_light.png `
  --shadow-output tmp/glock_shadow.png `
  --highlight-output tmp/glock_highlight.png `
  --table-shadow-output tmp/glock_table_shadow.png `
  --light-mode point `
  --light-pos 0.72,0.12,0.65
```

Useful knobs:

- `--light-mode`
- `--light-pos`
- `--light-falloff`
- `--ambient`
- `--directional`
- `--specular-strength`
- `--specular-power`
- `--shadow-strength`
- `--height-scale`
- `--light-dir`
- `--mask-erode`
- `--mask-blur`
- `--table-height-scale`
- `--thickness-scale`
- `--table-shadow-mode`
- `--table-shadow-opacity`
- `--table-shadow-blur-base`
- `--table-shadow-blur-height-scale`
- `--table-shadow-stretch-x`
- `--table-shadow-stretch-y`
- `--table-ground-offset-y`
- `--table-shadow-downsample`

## Clean Rebuild

```powershell
Remove-Item -Recurse -Force build
cmake -S . -B build -G Ninja
cmake --build build
```

## References

- MSYS2: https://www.msys2.org/
- MSYS2 CMake notes: https://www.msys2.org/docs/cmake/
- mingw-w64 MSYS2 setup: https://www.mingw-w64.org/getting-started/msys2/
