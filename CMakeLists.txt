cmake_minimum_required(VERSION 3.20)
project(SlimesSkyTravel VERSION 1.0.0 LANGUAGES C CXX)

# C++20を要求
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# コンパイルフラグ
if(MSVC)
    add_compile_definitions(NOMINMAX WIN32_LEAN_AND_MEAN)
    add_compile_options(/W4 /utf-8 /std:c++20)
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
if(WIN32)
    # WindowsではvcpkgでインストールしたSDL2とSDL2_mixerを使用
    find_package(SDL2 CONFIG REQUIRED)
    find_package(SDL2_mixer CONFIG REQUIRED)
    set(SDL2_MIXER_INCLUDE_DIRS ${SDL2_MIXER_INCLUDE_DIR})
else()
    # Linux/macOSではPkgConfigを使用
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(SDL2 REQUIRED sdl2)
    pkg_check_modules(SDL2_MIXER REQUIRED SDL2_mixer)
endif()

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

# 実行ファイルと同じ場所にassetsディレクトリをコピー（リリースビルド用）
add_custom_command(TARGET SlimesSkyTravel POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/assets
    $<TARGET_FILE_DIR:SlimesSkyTravel>/assets
    COMMENT "Copying assets directory to build output directory"
)

# インクルードディレクトリ
if(WIN32)
    target_include_directories(SlimesSkyTravel PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party
        ${SDL2_MIXER_INCLUDE_DIR}
    )
else()
    target_include_directories(SlimesSkyTravel PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party
        ${SDL2_MIXER_INCLUDE_DIRS}
    )
endif()

# ライブラリのリンク（共通）
target_link_libraries(SlimesSkyTravel
    glfw
    OpenGL::GL
    glm::glm
    nlohmann_json::nlohmann_json
)

# プラットフォーム固有のライブラリリンク
if(WIN32)
    # WindowsではvcpkgでインストールしたSDL2とSDL2_mixerをリンク
    target_link_libraries(SlimesSkyTravel
        $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
        $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
        $<IF:$<TARGET_EXISTS:SDL2_mixer::SDL2_mixer>,SDL2_mixer::SDL2_mixer,SDL2_mixer::SDL2_mixer-static>
    )
elseif(APPLE)
    # macOS固有の設定
    target_link_libraries(SlimesSkyTravel
        ${SDL2_MIXER_LIBRARIES}
        -L/opt/homebrew/Cellar/sdl2_mixer/2.8.1_1/lib -lSDL2_mixer
        -L/opt/homebrew/lib -lSDL2
        "-framework Cocoa"
        "-framework IOKit"
        "-framework CoreVideo"
    )
    target_compile_definitions(SlimesSkyTravel PRIVATE GL_SILENCE_DEPRECATION)
else()
    # Linux
    target_link_libraries(SlimesSkyTravel
        ${SDL2_LIBRARIES}
        ${SDL2_MIXER_LIBRARIES}
    )
endif()

# コンパイル定義
target_compile_definitions(SlimesSkyTravel PRIVATE
    $<$<CONFIG:Debug>:_DEBUG>
    $<$<CONFIG:Release>:NDEBUG>
)
