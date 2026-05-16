# ZombieGame

A pixel-art side-scrolling zombie survival shooter originally built with Win32/GDI.

## Recommended Windows Setup

Develop this project as a native Windows program. WSL2 is useful for many tools, but the current game uses Win32 APIs, Windows audio, and Windows-style resource paths, so native Windows builds are simpler.

The recommended lightweight stack is:

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

Optional but useful:

- Git for Windows: https://git-scm.com/download/win

## 2. Install MSYS2

Install MSYS2 from:

https://www.msys2.org/

Use the default install path unless you have a reason not to:

```text
C:\msys64
```

Open **MSYS2 UCRT64** from the Windows Start menu. Make sure it is the UCRT64 shell, not plain MSYS.

Update MSYS2 first:

```bash
pacman -Syu
```

If the shell asks you to close and reopen it, close it, open **MSYS2 UCRT64** again, then run:

```bash
pacman -Syu
```

## 3. Install The Toolchain

In **MSYS2 UCRT64**, run:

```bash
pacman -S --needed mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-gdb mingw-w64-ucrt-x86_64-cmake mingw-w64-ucrt-x86_64-ninja mingw-w64-ucrt-x86_64-pkgconf git
```

MSYS2 package names matter. For native Windows builds, prefer packages with the `mingw-w64-ucrt-x86_64-` prefix.

## 4. Add Tools To Windows PATH

Add this folder to your Windows user PATH:

```text
C:\msys64\ucrt64\bin
```

Then close and reopen PowerShell or VS Code.

Verify from a normal PowerShell terminal:

```powershell
gcc --version
g++ --version
gdb --version
cmake --version
ninja --version
```

If these commands are not found, PATH has not refreshed or `C:\msys64\ucrt64\bin` was not added correctly.

If you want to use Git from PowerShell or VS Code's Source Control panel, install Git for Windows as well. The `git` package installed inside MSYS2 is useful inside MSYS2 shells, but Git for Windows is the least surprising option for normal Windows terminals.

## 5. Open The Project

Open this folder in VS Code:

```text
D:\Code\game
```

Then build from the VS Code command palette:

```text
Tasks: Run Build Task
```

Or use the terminal commands below.

## Build From Terminal

From PowerShell in `D:\Code\game`:

```powershell
cmake -S . -B build -G Ninja -D CMAKE_BUILD_TYPE=Debug
cmake --build build
.\build\bin\zombie_game.exe
```

For a release build:

```powershell
cmake -S . -B build-release -G Ninja -D CMAKE_BUILD_TYPE=Release
cmake --build build-release
.\build-release\bin\zombie_game.exe
```

## Debug In VS Code

Use the included debug configuration:

```text
Run and Debug -> Debug zombie_game
```

This calls the default CMake build task first, then launches:

```text
build\bin\zombie_game.exe
```

## Common Setup Issues

### CMake Picks Visual Studio Instead Of Ninja

Pass the generator explicitly:

```powershell
cmake -S . -B build -G Ninja
```

If `build/` was already configured with a different generator, delete `build/` and configure again.

### Commands Work In MSYS2 But Not PowerShell

Add this to Windows PATH:

```text
C:\msys64\ucrt64\bin
```

Then restart PowerShell and VS Code.

### Wrong MSYS2 Shell

Use **MSYS2 UCRT64**. Avoid installing compiler packages into plain MSYS for this project.

### Resource Files Not Found

Run the executable with the project root as the working directory. The current game loads files such as:

```text
image\...
music\...
sound\...
save.dat
```

The VS Code launch configuration already sets `cwd` to the workspace folder.

## Current Build Target

The current target still uses the original Win32/GDI renderer. See `docs/ROADMAP.md` for the planned SDL3/raylib modernization path.

Useful commands:

```powershell
cmake --build build
.\build\bin\zombie_game.exe
```

Clean rebuild:

```powershell
Remove-Item -Recurse -Force build
cmake -S . -B build -G Ninja
cmake --build build
```

## References

- MSYS2: https://www.msys2.org/
- MSYS2 CMake notes: https://www.msys2.org/docs/cmake/
- mingw-w64 MSYS2 setup: https://www.mingw-w64.org/getting-started/msys2/
