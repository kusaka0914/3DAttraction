#pragma once

#include "game_state.h"
#include "platform_system.h"
#include <vector>
#include <functional>
#include <string>
#include <ctime>
#include <map>

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
    bool loadStage(int stageNumber, GameState& gameState, PlatformSystem& platformSystem);
    bool unlockStage(int stageNumber, GameState* gameState = nullptr);
    bool unlockStageWithStars(int stageNumber, int requiredStars, GameState& gameState);
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
    
    // ファイル監視（自動リロード用）
    bool checkAndReloadStage(GameState& gameState, PlatformSystem& platformSystem);
    
    // エディタ用：現在のステージファイルパスを取得
    std::string getCurrentStageFilePath() const { return currentStageFilePath; }
    
private:
    std::vector<StageData> stages;
    int currentStage;
    
    // ファイル監視用の変数
    std::string currentStageFilePath;  // 現在のステージのJSONファイルパス
    std::time_t lastFileModificationTime;  // 最後に確認した更新時刻
    std::map<int, std::string> stageFilePaths;  // ステージ番号とファイルパスのマッピング
    
    // ファイル監視用のヘルパー関数
    std::time_t getFileModificationTime(const std::string& filepath);
    std::string getStageFilePath(int stageNumber);
    void updateCurrentStageFileInfo(int stageNumber);
    
    // 各ステージの生成関数
    static void generateStageSelectionField(GameState& gameState, PlatformSystem& platformSystem);
    static void generateStage1(GameState& gameState, PlatformSystem& platformSystem);
    static void generateStage2(GameState& gameState, PlatformSystem& platformSystem);
    static void generateStage3(GameState& gameState, PlatformSystem& platformSystem);
    static void generateStage4(GameState& gameState, PlatformSystem& platformSystem);
    static void generateStage5(GameState& gameState, PlatformSystem& platformSystem);
    static void generateTutorialStage(GameState& gameState, PlatformSystem& platformSystem);
    
    // 設定ファイルからステージを生成する関数
    bool generateStageFromConfig(int stageNumber, GameState& gameState, PlatformSystem& platformSystem);
};
