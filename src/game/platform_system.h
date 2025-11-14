#pragma once

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "game_state.h"
#include "../core/types/platform_types.h"
#include <vector>
#include <functional>

/**
 * @brief プラットフォームシステム
 * @details 分離型設計のプラットフォームシステム。各種プラットフォームタイプを統合的に管理します。
 */
class PlatformSystem {
private:
    std::vector<PlatformVariant> platforms;
    
public:
    /**
     * @brief プラットフォームを追加する
     * @details テンプレート関数で任意のプラットフォームタイプを追加できます。
     * 
     * @tparam T プラットフォームタイプ
     * @param platform 追加するプラットフォーム
     */
    template<typename T>
    void addPlatform(const T& platform) {
        platforms.push_back(platform);
    }
    
    /**
     * @brief プラットフォームを更新する
     * @details 全プラットフォームの状態を更新します。
     * 
     * @param deltaTime デルタタイム
     * @param playerPos プレイヤー位置（オプション）
     */
    void update(float deltaTime, const glm::vec3& playerPos = glm::vec3(0));
    
    /**
     * @brief 衝突判定（インデックス付き）
     * @details プレイヤーとプラットフォームの衝突判定を行い、衝突したプラットフォームとインデックスを返します。
     * 
     * @param playerPos プレイヤー位置
     * @param playerSize プレイヤーサイズ
     * @return 衝突したプラットフォームへのポインタとインデックスのペア
     */
    std::pair<PlatformVariant*, int> checkCollisionWithIndex(const glm::vec3& playerPos, const glm::vec3& playerSize);
    
    /**
     * @brief 全てのプラットフォームを取得する（const版）
     * @return プラットフォームのベクターへのconst参照
     */
    const std::vector<PlatformVariant>& getPlatforms() const { return platforms; }
    
    /**
     * @brief 全てのプラットフォームを取得する（非const版）
     * @details エディタ用に非const版も提供します。
     * 
     * @return プラットフォームのベクターへの参照
     */
    std::vector<PlatformVariant>& getPlatforms() { return platforms; }
    
    /**
     * @brief プラットフォームをクリアする
     * @details 全プラットフォームを削除します。
     */
    void clear() { platforms.clear(); }
    
    /**
     * @brief プラットフォームを削除する
     * @details エディタ用に指定インデックスのプラットフォームを削除します。
     * 
     * @param index 削除するプラットフォームのインデックス
     * @return 削除成功時true
     */
    bool removePlatform(int index) {
        if (index >= 0 && index < static_cast<int>(platforms.size())) {
            platforms.erase(platforms.begin() + index);
            return true;
        }
        return false;
    }
    
    /**
     * @brief プラットフォームの位置を取得する
     * @return 位置のベクター
     */
    std::vector<glm::vec3> getPositions() const;
    
    /**
     * @brief プラットフォームのサイズを取得する
     * @return サイズのベクター
     */
    std::vector<glm::vec3> getSizes() const;
    
    /**
     * @brief プラットフォームの色を取得する
     * @return 色のベクター
     */
    std::vector<glm::vec3> getColors() const;
    
    /**
     * @brief プラットフォームの可視性を取得する
     * @return 可視性のベクター
     */
    std::vector<bool> getVisibility() const;
    
    /**
     * @brief プラットフォームの回転状態を取得する
     * @return 回転状態のベクター
     */
    std::vector<bool> getIsRotating() const;
    
    /**
     * @brief プラットフォームの回転角度を取得する
     * @return 回転角度のベクター
     */
    std::vector<float> getRotationAngles() const;
    
    /**
     * @brief プラットフォームの回転軸を取得する
     * @return 回転軸のベクター
     */
    std::vector<glm::vec3> getRotationAxes() const;
    
    /**
     * @brief プラットフォームの点滅アルファ値を取得する
     * @return 点滅アルファ値のベクター
     */
    std::vector<float> getBlinkAlphas() const;
    
    /**
     * @brief プラットフォームタイプを取得する
     * @details テクスチャ描画用にプラットフォームタイプの文字列を取得します。
     * 
     * @return プラットフォームタイプの文字列ベクター
     */
    std::vector<std::string> getPlatformTypes() const;
    
    /**
     * @brief MovingPlatformのhasPlayerOnBoardフラグをリセットする
     * @details 全MovingPlatformのhasPlayerOnBoardフラグをfalseにリセットします。
     */
    void resetMovingPlatformFlags();
    
private:
    void updateStaticPlatform(StaticPlatform& platform, float deltaTime);
    void updateMovingPlatform(MovingPlatform& platform, float deltaTime);
    void updateRotatingPlatform(RotatingPlatform& platform, float deltaTime);
    void updatePatrollingPlatform(PatrollingPlatform& platform, float deltaTime);
    void updateTeleportPlatform(TeleportPlatform& platform, float deltaTime);
    void updateJumpPad(JumpPad& platform, float deltaTime);
    void updateCycleDisappearingPlatform(CycleDisappearingPlatform& platform, float deltaTime);
    void updateDisappearingPlatform(DisappearingPlatform& platform, float deltaTime);
    void updateFlyingPlatform(FlyingPlatform& platform, float deltaTime, const glm::vec3& playerPos);
    
    bool checkCollisionWithBase(const BasePlatform& platform, const glm::vec3& playerPos, const glm::vec3& playerSize);
    bool checkCollisionWithRotatingPlatform(const RotatingPlatform& platform, const glm::vec3& playerPos, const glm::vec3& playerSize);
};

/**
 * @brief バリアント訪問用のヘルパー構造体
 * @details std::visitで使用するオーバーロード構造体です。
 */
template<typename... Ts>
struct overloaded : Ts... { using Ts::operator()...; };

template<typename... Ts>
overloaded(Ts...) -> overloaded<Ts...>;
