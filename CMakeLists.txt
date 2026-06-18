cmake_minimum_required(VERSION 3.20)

project(ZombieGame LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(SDL3 CONFIG REQUIRED)
find_package(SDL3_image CONFIG REQUIRED)
find_package(SDL3_ttf CONFIG REQUIRED)

add_executable(zombie_game WIN32
    src/AssetPaths.cpp
    src/Assets.cpp
    src/Bullet.cpp
    src/Camera.cpp
    src/CollisionMap.cpp
    src/Effects.cpp
    src/Grenade.cpp
    src/HudRenderer.cpp
    src/HumanoidBody.cpp
    src/Input.cpp
    src/InventoryState.cpp
    src/LocalizationTable.cpp
    src/Player.cpp
    src/Renderer2D.cpp
    src/SoundSystem.cpp
    src/SpriteCatalog.cpp
    src/SpriteSheet.cpp
    src/SurfaceMask.cpp
    src/Texture.cpp
    src/Weapon.cpp
    src/WeaponRelight.cpp
    src/Zombie.cpp
    src/ZombieDirector.cpp
    src/ui/Button.cpp
    src/ui/Card.cpp
    src/ui/ControlStyle.cpp
    src/ui/GameHudLayout.cpp
    src/ui/InventoryLayout.cpp
    src/ui/InventoryScreen.cpp
    src/ui/LayoutJson.cpp
    src/ui/ListView.cpp
    src/ui/Panel.cpp
    src/ui/ProgressBar.cpp
    src/ui/Scrollbar.cpp
    src/ui/WorkbenchLayout.cpp
    src/ui/WorkbenchScreen.cpp
    src/main.cpp
)

target_link_libraries(zombie_game PRIVATE
    SDL3::SDL3
    SDL3_image::SDL3_image
    SDL3_ttf::SDL3_ttf
)

set_target_properties(zombie_game PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
