#pragma once

#include "game_state.h"
#include "platform_system.h"
#include <vector>
#include <functional>

// ステージデータの構造体
struct StageData {
    int stageNumber;
    std::string stageName;
    glm::vec3 playerStartPosition;
    glm::vec3 goalPosition;
    std::function<void(GameState&, PlatformSystem&)> generateFunction;
    bool isUnlocked;
    bool isCompleted;
    float timeLimit;  // ステージ固有の制限時間（秒）
};

// ステージ管理システム
class StageManager {
public:
    StageManager();
    ~StageManager() = default;
    
    // ステージ管理
    void initializeStages();
    void loadStage(int stageNumber, GameState& gameState, PlatformSystem& platformSystem);
    void unlockStage(int stageNumber);
    void completeStage(int stageNumber);
    void resetStageProgress();
    
    // ステージ情報取得
    int getCurrentStage() const { return currentStage; }
    int getTotalStages() const { return stages.size(); }
    bool isStageUnlocked(int stageNumber) const;
    bool isStageCompleted(int stageNumber) const;
    const StageData* getStageData(int stageNumber) const;
    
    // 星数管理
    int getStageStars(int stageNumber) const;
    int getTotalStars() const;
    void updateStageStars(int stageNumber, int newStars);
    int calculateStarDifference(int stageNumber, int newStars) const;
    
    // ステージ移動
    bool goToNextStage(GameState& gameState, PlatformSystem& platformSystem);
    bool goToPreviousStage(GameState& gameState, PlatformSystem& platformSystem);
    bool goToStage(int stageNumber, GameState& gameState, PlatformSystem& platformSystem);
    
private:
    std::vector<StageData> stages;
    int currentStage;
    
    // 各ステージの生成関数
    static void generateStageSelectionField(GameState& gameState, PlatformSystem& platformSystem);
    static void generateStage1(GameState& gameState, PlatformSystem& platformSystem);
    static void generateStage2(GameState& gameState, PlatformSystem& platformSystem);
    static void generateStage3(GameState& gameState, PlatformSystem& platformSystem);
    static void generateStage4(GameState& gameState, PlatformSystem& platformSystem);
    static void generateStage5(GameState& gameState, PlatformSystem& platformSystem);
};
