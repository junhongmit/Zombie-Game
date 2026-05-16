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
