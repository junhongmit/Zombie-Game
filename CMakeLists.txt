cmake_minimum_required(VERSION 3.20)

project(ZombieGame LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(zombie_game WIN32
    AI.cpp
    blood.cpp
    main.cpp
    save.cpp
    smog.cpp
    sound.cpp
    useful.cpp
    window.cpp
)

target_compile_definitions(zombie_game PRIVATE
    WIN32_LEAN_AND_MEAN
)

if(MSVC)
    target_compile_options(zombie_game PRIVATE /W3 /permissive-)
else()
    target_compile_options(zombie_game PRIVATE -Wall -Wextra)
endif()

target_link_libraries(zombie_game PRIVATE
    gdi32
    user32
    kernel32
    comctl32
    winmm
    msimg32
    gdiplus
)

set_target_properties(zombie_game PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

find_package(SDL3 CONFIG REQUIRED)
find_package(SDL3_image CONFIG REQUIRED)

add_executable(zombie_sdl3 WIN32
    src/sdl3/Assets.cpp
    src/sdl3/Camera.cpp
    src/sdl3/Input.cpp
    src/sdl3/Player.cpp
    src/sdl3/Renderer2D.cpp
    src/sdl3/Texture.cpp
    src/sdl3/Zombie.cpp
    src/sdl3/main.cpp
)

target_link_libraries(zombie_sdl3 PRIVATE
    SDL3::SDL3
    SDL3_image::SDL3_image
)

set_target_properties(zombie_sdl3 PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
