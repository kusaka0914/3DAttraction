#pragma once

#include "game_state.h"
#include "platform_system.h"
#include <vector>
#include <functional>
#include <string>
#include <ctime>
#include <map>

/**
 * @brief ステージデータの構造体
 * @details ステージの基本情報を保持します。
 */
struct StageData {
    int stageNumber;
    std::string stageName;
    glm::vec3 playerStartPosition;
    glm::vec3 goalPosition;
    std::function<void(GameState&, PlatformSystem&)> generateFunction;
    bool isUnlocked;
    bool isCompleted;
    float timeLimit;
};

/**
 * @brief ステージ管理システム
 * @details ステージの読み込み、解放、進行状況の管理を行います。
 */
class StageManager {
public:
    StageManager();
    ~StageManager() = default;
    
    /**
     * @brief ステージを初期化する
     * @details 全ステージのデータを初期化します。
     */
    void initializeStages();
    
    /**
     * @brief ステージを読み込む
     * @details 指定されたステージ番号のステージを読み込みます。
     * 
     * @param stageNumber ステージ番号
     * @param gameState ゲーム状態
     * @param platformSystem プラットフォームシステム
     * @return 読み込み成功時true
     */
    bool loadStage(int stageNumber, GameState& gameState, PlatformSystem& platformSystem);
    
    /**
     * @brief ステージを解放する
     * @details 指定されたステージ番号のステージを解放します。
     * 
     * @param stageNumber ステージ番号
     * @param gameState ゲーム状態（オプション）
     * @return 解放成功時true
     */
    bool unlockStage(int stageNumber, GameState* gameState = nullptr);
    
    /**
     * @brief 星数でステージを解放する
     * @details 必要な星数が揃っている場合、ステージを解放します。
     * 
     * @param stageNumber ステージ番号
     * @param requiredStars 必要な星数
     * @param gameState ゲーム状態
     * @return 解放成功時true
     */
    bool unlockStageWithStars(int stageNumber, int requiredStars, GameState& gameState);
    
    /**
     * @brief ステージをクリア済みにする
     * @details 指定されたステージ番号のステージをクリア済みにマークします。
     * 
     * @param stageNumber ステージ番号
     */
    void completeStage(int stageNumber);
    
    /**
     * @brief ステージ進行状況をリセットする
     * @details 全ステージの進行状況をリセットします。
     */
    void resetStageProgress();
    
    /**
     * @brief 現在のステージ番号を取得する
     * @return 現在のステージ番号
     */
    int getCurrentStage() const { return currentStage; }
    
    /**
     * @brief 総ステージ数を取得する
     * @return 総ステージ数
     */
    int getTotalStages() const { return stages.size(); }
    
    /**
     * @brief ステージが解放されているか確認する
     * @param stageNumber ステージ番号
     * @return 解放されている場合true
     */
    bool isStageUnlocked(int stageNumber) const;
    
    /**
     * @brief ステージがクリア済みか確認する
     * @param stageNumber ステージ番号
     * @return クリア済みの場合true
     */
    bool isStageCompleted(int stageNumber) const;
    
    /**
     * @brief ステージデータを取得する
     * @param stageNumber ステージ番号
     * @return ステージデータへのポインタ（存在しない場合はnullptr）
     */
    const StageData* getStageData(int stageNumber) const;
    
    /**
     * @brief ステージの星数を取得する
     * @param stageNumber ステージ番号
     * @return 星数
     */
    int getStageStars(int stageNumber) const;
    
    /**
     * @brief 総星数を取得する
     * @return 総星数
     */
    int getTotalStars() const;
    
    /**
     * @brief ステージの星数を更新する
     * @param stageNumber ステージ番号
     * @param newStars 新しい星数
     */
    void updateStageStars(int stageNumber, int newStars);
    
    /**
     * @brief 星数の差分を計算する
     * @param stageNumber ステージ番号
     * @param newStars 新しい星数
     * @return 星数の差分
     */
    int calculateStarDifference(int stageNumber, int newStars) const;
    
    /**
     * @brief 次のステージに移動する
     * @details 次のステージに移動します。
     * 
     * @param gameState ゲーム状態
     * @param platformSystem プラットフォームシステム
     * @return 移動成功時true
     */
    bool goToNextStage(GameState& gameState, PlatformSystem& platformSystem);
    
    /**
     * @brief 前のステージに移動する
     * @details 前のステージに移動します。
     * 
     * @param gameState ゲーム状態
     * @param platformSystem プラットフォームシステム
     * @return 移動成功時true
     */
    bool goToPreviousStage(GameState& gameState, PlatformSystem& platformSystem);
    
    /**
     * @brief 指定ステージに移動する
     * @details 指定されたステージ番号のステージに移動します。
     * 
     * @param stageNumber ステージ番号
     * @param gameState ゲーム状態
     * @param platformSystem プラットフォームシステム
     * @return 移動成功時true
     */
    bool goToStage(int stageNumber, GameState& gameState, PlatformSystem& platformSystem);
    
    /**
     * @brief ステージファイルの変更を確認してリロードする
     * @details ステージファイルが変更されていた場合、自動的にリロードします。
     * 
     * @param gameState ゲーム状態
     * @param platformSystem プラットフォームシステム
     * @return リロードが発生した場合true
     */
    bool checkAndReloadStage(GameState& gameState, PlatformSystem& platformSystem);
    
    /**
     * @brief 現在のステージファイルパスを取得する
     * @details エディタ用に現在のステージのJSONファイルパスを取得します。
     * 
     * @return 現在のステージファイルパス
     */
    std::string getCurrentStageFilePath() const { return currentStageFilePath; }
    
private:
    std::vector<StageData> stages;
    int currentStage;
    
    std::string currentStageFilePath;
    std::time_t lastFileModificationTime;
    std::map<int, std::string> stageFilePaths;
    
    std::time_t getFileModificationTime(const std::string& filepath);
    std::string getStageFilePath(int stageNumber);
    void updateCurrentStageFileInfo(int stageNumber);
    
    static void generateStageSelectionField(GameState& gameState, PlatformSystem& platformSystem);
    static void generateStage1(GameState& gameState, PlatformSystem& platformSystem);
    static void generateStage2(GameState& gameState, PlatformSystem& platformSystem);
    static void generateStage3(GameState& gameState, PlatformSystem& platformSystem);
    static void generateStage4(GameState& gameState, PlatformSystem& platformSystem);
    static void generateStage5(GameState& gameState, PlatformSystem& platformSystem);
    static void generateTutorialStage(GameState& gameState, PlatformSystem& platformSystem);
    
    bool generateStageFromConfig(int stageNumber, GameState& gameState, PlatformSystem& platformSystem);
};
