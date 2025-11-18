/**
 * @file ui_state.h
 * @brief UIの状態を管理する構造体
 * @details タイトル画面、ステージクリア、ステージ選択、チュートリアル、エンディングなどのUI表示状態を保持します。
 */
#pragma once

#include <string>
#include <vector>
#include <glm/glm.hpp>
#include "online_leaderboard_manager.h"

/**
 * @brief UIの状態を管理する構造体
 * @details タイトル画面、ステージクリア、ステージ選択、チュートリアル、エンディングなどのUI表示状態を保持します。
 */
struct UIState {
    bool showStageClearUI = false;
    float stageClearTimer = 0.0f;
    bool stageClearConfirmed = false;
    
    bool showUnlockConfirmUI = false;
    int unlockTargetStage = 0;
    int unlockRequiredStars = 0;
    
    bool showStarInsufficientUI = false;
    int insufficientTargetStage = 0;
    int insufficientRequiredStars = 0;
    
    bool showWarpTutorialUI = false;
    int warpTutorialStage = 0;
    bool blockEnterUntilReleased = false;
    
    bool showStageSelectionAssist = false;
    int assistTargetStage = 0;
    
    bool showLeaderboardAssist = false;  /**< @brief ランキングボードアシスト表示フラグ */
    bool showLeaderboardUI = false;  /**< @brief ランキングUI表示フラグ */
    int leaderboardTargetStage = 0;  /**< @brief ランキング表示対象ステージ */
    std::vector<LeaderboardEntry> leaderboardEntries;  /**< @brief ランキングエントリ */
    bool isLoadingLeaderboard = false;  /**< @brief ランキング読み込み中フラグ */
    float leaderboardRetryTimer = 0.0f;  /**< @brief ランキングリトライタイマー（秒） */
    int leaderboardRetryCount = 0;  /**< @brief ランキングリトライ回数 */
    glm::vec3 leaderboardPosition = glm::vec3(0.0f, 1.0f, 0.0f);  /**< @brief ランキングボードの位置（JSONから読み込む） */
    
    bool showStage0Tutorial = true;
    
    bool showTitleScreen = true;
    float titleScreenTimer = 0.0f;
    enum class TitleScreenPhase {
        BACKGROUND_FADE_IN,
        LOGO_ANIMATION,
        SHOW_TEXT
    };
    TitleScreenPhase titleScreenPhase = TitleScreenPhase::BACKGROUND_FADE_IN;
    
    bool showPlayerNameInput = false;  /**< @brief プレイヤー名入力画面表示フラグ */
    std::string playerNameInput = "";  /**< @brief プレイヤー名入力中の文字列 */
    int playerNameInputCursorPos = 0;  /**< @brief プレイヤー名入力カーソル位置 */
    
    bool isTransitioning = false;
    float transitionTimer = 0.0f;
    enum class TransitionType {
        NONE,
        FADE_OUT,
        FADE_IN
    };
    TransitionType transitionType = TransitionType::NONE;
    int pendingStageTransition = -1;
    bool pendingReadyScreen = false;
    bool pendingSkipCountdown = false;
    bool pendingFadeIn = false;  // フェードアウト完了後、次のフレームでフェードインを開始するためのフラグ
    bool stageLoadedForFadeIn = false;  // ステージが読み込まれた後、描画が完了するまで待つためのフラグ
    
    bool showEasyModeExplanationUI = false;
    
    bool showModeSelectionUI = false;
    bool showTimeAttackSelectionUI = false;
    int modeSelectionTargetStage = 0;
    
    bool showSecretStarExplanationUI = false;
    bool showSecretStarSelectionUI = false;
    
    bool showReadyScreen = false;
    bool readyScreenShown = false;
    int readyScreenSpeedLevel = 0;
    bool isCountdownActive = false;
    float countdownTimer = 3.0f;
    
    bool showTutorialUI = false;
    std::string tutorialMessage = "";
    
    bool showStaffRoll = false;
    bool showEndingMessage = false;
    float staffRollTimer = 0.0f;
    float endingMessageTimer = 0.0f;
    bool isEndingSequence = false;
};

