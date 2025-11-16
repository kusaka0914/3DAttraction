/**
 * @file game_progress_state.h
 * @brief ゲーム進行の状態を管理する構造体
 * @details ステージ進行、タイムアタック、SECRET STAR、チュートリアル、EASYモードなどのゲーム進行に関する状態を保持します。
 */
#pragma once

#include <map>
#include <set>
#include <string>
#include <vector>
#include <glm/glm.hpp>

/**
 * @brief ゲーム進行の状態を管理する構造体
 * @details ステージ進行、タイムアタック、SECRET STAR、チュートリアル、EASYモードなどのゲーム進行に関する状態を保持します。
 */
struct GameProgressState {
    float gameTime = 0.0f;
    int currentStage = 0;
    
    float timeLimit = 20.0f;
    float remainingTime = 20.0f;
    int earnedStars = 0;
    float clearTime = 0.0f;
    bool isTimeUp = false;
    bool isStageCompleted = false;
    
    std::map<int, int> stageStars;
    int totalStars = 0;
    std::map<int, bool> unlockedStages;
    
    bool isGameCleared = false;
    bool hasShownSecretStarExplanationUI = false;  /**< @brief シークレットスター説明UIを表示済みか */
    bool isGameOver = false;
    float gameOverTimer = 0.0f;
    int lives = 6;
    
    bool isGoalReached = false;
    bool gameWon = false;
    glm::vec3 goalPosition = glm::vec3(0, 2.0f, 20);
    glm::vec3 goalColor = glm::vec3(1, 1, 0);
    
    bool isTimeAttackMode = false;
    float currentTimeAttackTime = 0.0f;
    std::map<int, float> timeAttackRecords;
    bool isNewRecord = false;
    float timeAttackStartTime = 0.0f;
    
    enum class SecretStarType {
        NONE,
        MAX_SPEED_STAR,
        SHADOW_STAR,
        IMMERSIVE_STAR
    };
    SecretStarType selectedSecretStarType = SecretStarType::MAX_SPEED_STAR;
    std::map<int, std::set<SecretStarType>> secretStarCleared;
    
    bool isTutorialStage = false;
    int tutorialStep = 0;
    bool tutorialStepCompleted = false;
    glm::vec3 tutorialStartPosition = glm::vec3(0, 0, 0);
    float tutorialRequiredDistance = 5.0f;
    bool tutorialInputEnabled = false;
    
    bool isEasyMode = false;
    float timeScale = 1.0f;
    int timeScaleLevel = 0;
    bool timeLimitApplied = false;
};

