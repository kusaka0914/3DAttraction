cmake_minimum_required(VERSION 3.20)
project(Vulkan3D VERSION 1.0.0 LANGUAGES C CXX)

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

# メインアプリケーション
add_executable(Vulkan3D 
    src/app/main.cpp
    src/gfx/opengl_renderer.cpp
    src/gfx/bitmap_font.cpp
    src/gfx/background_renderer.cpp
    src/gfx/ui_renderer.cpp
    src/gfx/game_state_ui_renderer.cpp
    src/gfx/renderer_3d.cpp
    src/game/game_state.cpp
    src/game/stage_manager.cpp
    src/game/platform_system.cpp
    src/game/cannon_system.cpp
    src/game/switch_system.cpp
    src/game/gravity_system.cpp
    src/physics/physics_system.cpp
    src/io/input_system.cpp
    src/core/utils/physics_utils.cpp
    src/core/utils/stage_utils.cpp
)

# インクルードディレクトリ
target_include_directories(Vulkan3D PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# ライブラリのリンク
target_link_libraries(Vulkan3D
    glfw
    OpenGL::GL
    glm::glm
)

# プラットフォーム固有の設定
if(APPLE)
    target_link_libraries(Vulkan3D
        "-framework Cocoa"
        "-framework IOKit"
        "-framework CoreVideo"
    )
    target_compile_definitions(Vulkan3D PRIVATE GL_SILENCE_DEPRECATION)
endif()

# コンパイル定義
target_compile_definitions(Vulkan3D PRIVATE
    $<$<CONFIG:Debug>:_DEBUG>
    $<$<CONFIG:Release>:NDEBUG>
)
