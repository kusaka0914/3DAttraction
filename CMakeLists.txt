cmake_minimum_required(VERSION 3.20)
project(SlimesSkyTravel VERSION 1.0.0 LANGUAGES C CXX)

# C++20を要求
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# コンパイラフラグ
if(MSVC)
    add_compile_options(/W4 /utf-8)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# GLFW
find_package(glfw3 REQUIRED)

# GLM
find_package(glm REQUIRED)

# OpenGL
find_package(OpenGL REQUIRED)

# JSON library (nlohmann/json)
find_package(nlohmann_json REQUIRED)

# STB Image library for texture loading (optional)
# find_package(stb REQUIRED)

# Audio libraries - using SDL2_mixer for cross-platform audio
find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2_MIXER REQUIRED SDL2_mixer)

# メインアプリケーション
add_executable(SlimesSkyTravel
    src/app/main.cpp
    src/app/game_loop.cpp
    src/app/tutorial_manager.cpp
    src/gfx/opengl_renderer.cpp
    src/gfx/bitmap_font.cpp
    src/gfx/background_renderer.cpp
    src/gfx/ui_renderer.cpp
    src/gfx/game_state_ui_renderer.cpp
    src/gfx/renderer_3d.cpp
    src/gfx/texture_manager.cpp
    src/game/game_state.cpp
    src/game/stage_manager.cpp
    src/game/platform_system.cpp
    src/game/json_stage_loader.cpp
    src/game/cannon_system.cpp
    src/game/switch_system.cpp
    src/game/gravity_system.cpp
    src/physics/physics_system.cpp
    src/io/input_system.cpp
    src/io/audio_manager.cpp
    src/core/utils/physics_utils.cpp
    src/core/utils/stage_utils.cpp
)

# 実行ファイル名を元の名前に設定
set_target_properties(SlimesSkyTravel PROPERTIES
    OUTPUT_NAME "SlimesSkyTravel"
)

# インクルードディレクトリ
target_include_directories(SlimesSkyTravel PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party
    ${SDL2_MIXER_INCLUDE_DIRS}
)

# ライブラリのリンク
target_link_libraries(SlimesSkyTravel
    glfw
    OpenGL::GL
    glm::glm
    nlohmann_json::nlohmann_json
    ${SDL2_MIXER_LIBRARIES}
    -L/opt/homebrew/Cellar/sdl2_mixer/2.8.1_1/lib -lSDL2_mixer
    -L/opt/homebrew/lib -lSDL2
    # stb::stb
)

# プラットフォーム固有の設定
if(APPLE)
    target_link_libraries(SlimesSkyTravel
        "-framework Cocoa"
        "-framework IOKit"
        "-framework CoreVideo"
    )
    target_compile_definitions(SlimesSkyTravel PRIVATE GL_SILENCE_DEPRECATION)
endif()

# コンパイル定義
target_compile_definitions(SlimesSkyTravel PRIVATE
    $<$<CONFIG:Debug>:_DEBUG>
    $<$<CONFIG:Release>:NDEBUG>
)
