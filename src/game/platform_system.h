#pragma once

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "game_state.h"
#include "../core/types/platform_types.h"
#include <vector>
#include <functional>

// 新しい分離型設計のプラットフォームシステム
class PlatformSystem {
private:
    std::vector<PlatformVariant> platforms;
    
public:
    // プラットフォームの追加
    template<typename T>
    void addPlatform(const T& platform) {
        platforms.push_back(platform);
    }
    
    // プラットフォームの更新
    void update(float deltaTime, const glm::vec3& playerPos = glm::vec3(0));
    
    // 衝突判定（インデックス付き）
    std::pair<PlatformVariant*, int> checkCollisionWithIndex(const glm::vec3& playerPos, const glm::vec3& playerSize);
    
    // 全てのプラットフォームの取得
    const std::vector<PlatformVariant>& getPlatforms() const { return platforms; }
    
    // プラットフォームのクリア
    void clear() { platforms.clear(); }
    
    // プラットフォームの描画用データ取得
    std::vector<glm::vec3> getPositions() const;
    std::vector<glm::vec3> getSizes() const;
    std::vector<glm::vec3> getColors() const;
    std::vector<bool> getVisibility() const;
    std::vector<bool> getIsRotating() const;
    std::vector<float> getRotationAngles() const;
    std::vector<glm::vec3> getRotationAxes() const;
    std::vector<float> getBlinkAlphas() const;
    
    // 足場タイプの取得（テクスチャ描画用）
    std::vector<std::string> getPlatformTypes() const;
    
    // MovingPlatformのhasPlayerOnBoardフラグをリセット
    void resetMovingPlatformFlags();
    
private:
    // 各足場タイプの更新処理
    void updateStaticPlatform(StaticPlatform& platform, float deltaTime);
    void updateMovingPlatform(MovingPlatform& platform, float deltaTime);
    void updateRotatingPlatform(RotatingPlatform& platform, float deltaTime);
    void updatePatrollingPlatform(PatrollingPlatform& platform, float deltaTime);
    void updateTeleportPlatform(TeleportPlatform& platform, float deltaTime);
    void updateJumpPad(JumpPad& platform, float deltaTime);
    void updateCycleDisappearingPlatform(CycleDisappearingPlatform& platform, float deltaTime);
    void updateDisappearingPlatform(DisappearingPlatform& platform, float deltaTime);
    void updateFlyingPlatform(FlyingPlatform& platform, float deltaTime, const glm::vec3& playerPos);
    
    // 衝突判定ヘルパー
    bool checkCollisionWithBase(const BasePlatform& platform, const glm::vec3& playerPos, const glm::vec3& playerSize);
    bool checkCollisionWithRotatingPlatform(const RotatingPlatform& platform, const glm::vec3& playerPos, const glm::vec3& playerSize);
};

// バリアント訪問用のヘルパー関数
template<typename... Ts>
struct overloaded : Ts... { using Ts::operator()...; };

template<typename... Ts>
overloaded(Ts...) -> overloaded<Ts...>;
