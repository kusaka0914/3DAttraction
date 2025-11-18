/**
 * @file physics_constants.h
 * @brief 物理定数
 * @details 物理演算に関する定数を定義します。
 */
#pragma once

#include <glm/glm.hpp>

namespace GameConstants {
    // 物理設定
    constexpr float BASE_GRAVITY = 12.0f;
    constexpr float AIR_RESISTANCE_NORMAL = 0.98f;
    constexpr float AIR_RESISTANCE_FAST = 0.99f;
    
    // プレイヤー設定
    constexpr glm::vec3 PLAYER_SIZE = glm::vec3(1.0f, 1.0f, 1.0f);
    constexpr float PLAYER_SCALE = 0.5f;
    
    // 物理システム設定
    namespace PhysicsConstants {
        // 衝突判定設定
        constexpr float COLLISION_TOLERANCE = 0.1f;
        constexpr float PLATFORM_COLLISION_MARGIN = 0.1f;
        constexpr float GRAVITY_DIRECTION_THRESHOLD = 0.5f;
        
        // プレイヤー設定
        constexpr float PLAYER_COLLISION_MARGIN = 0.1f;
        constexpr float PLATFORM_SURFACE_TOLERANCE = 0.1f;
    }
    
    // 物理計算設定
    namespace PhysicsCalculationConstants {
        // 重力計算設定
        constexpr float GRAVITY_MULTIPLIER_STAGE_0 = 2.0f;
        constexpr float GRAVITY_MULTIPLIER_TIME_SCALE = 1.0f;
        constexpr float GRAVITY_MULTIPLIER_INVERTED = 0.7f;
        constexpr float GRAVITY_MULTIPLIER_BURST_JUMP = 0.5f;
        constexpr float GRAVITY_ACCELERATION = 9.8f;
        
        // 衝突判定設定
        constexpr float PLATFORM_HALF_SIZE_MULTIPLIER = 0.5f;
        constexpr float PLAYER_HALF_SIZE_MULTIPLIER = 0.5f;
        constexpr float PLATFORM_COLLISION_DISTANCE = 0.5f;
        constexpr float PLATFORM_COLLISION_DISTANCE_STRICT = 0.1f;
    }
}
