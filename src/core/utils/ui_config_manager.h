#pragma once

#include <string>
#include <glm/glm.hpp>
#include <map>

namespace UIConfig {
    struct UIPosition {
        float offsetX = 0.0f;  // 画面中央からのオフセットX（相対位置）
        float offsetY = 0.0f;  // 画面中央からのオフセットY（相対位置）
        float absoluteX = 0.0f;  // 絶対位置X（画面左上基準）
        float absoluteY = 0.0f;  // 絶対位置Y（画面左上基準）
        bool useRelative = true;  // true: 相対位置を使用、false: 絶対位置を使用
    };
    
    struct UITextConfig {
        UIPosition position;
        glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f);
        glm::vec3 completedColor = glm::vec3(1.0f, 1.0f, 1.0f);  // 完了時の色（チュートリアルメッセージ用）
        float scale = 1.0f;
    };
    
    struct UISelectableConfig {
        UIPosition position;
        glm::vec3 selectedColor = glm::vec3(1.0f, 0.8f, 0.2f);
        glm::vec3 unselectedColor = glm::vec3(0.5f, 0.5f, 0.5f);
        float scale = 1.0f;
        float spacing = 0.0f;  // 複数要素間の間隔（speedOptions用）
    };
    
    struct UITimeDisplayConfig {
        UIPosition position;
        glm::vec3 normalColor = glm::vec3(1.0f, 1.0f, 1.0f);
        glm::vec3 warningColor = glm::vec3(1.0f, 0.5f, 0.0f);
        float scale = 1.0f;
    };
    
    struct UISkillConfig {
        UIPosition position;
        float countOffset = 0.0f;
        float instructionOffset = 0.0f;
        glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f);
        glm::vec3 disabledColor = glm::vec3(0.5f, 0.5f, 0.5f);
        glm::vec3 activeColor = glm::vec3(0.5f, 0.5f, 1.0f);
        float scale = 1.0f;
        float countScale = 1.0f;
        float instructionScale = 1.0f;
        UIPosition activePosition;  // freeCamera用
        float activeScale = 1.0f;   // freeCamera用
    };
    
    class UIConfigManager {
    public:
        static UIConfigManager& getInstance();
        
        bool loadConfig(const std::string& filepath = "assets/config/ui_config.json");
        void reloadConfig();
        bool checkAndReloadConfig();  // ファイル変更をチェックして再読み込み
        
        // UI要素の設定を取得
        UITextConfig getStageInfoConfig() const { return stageInfoConfig; }
        UITextConfig getSpeedDisplayConfig() const { return speedDisplayConfig; }
        UITextConfig getPressTConfig() const { return pressTConfig; }
        UITextConfig getReplayPauseMarkConfig() const { return replayPauseMarkConfig; }
        UITextConfig getReplayRewindMarkConfig() const { return replayRewindMarkConfig; }
        UITextConfig getReplayFastForwardMarkConfig() const { return replayFastForwardMarkConfig; }
        UITextConfig getReplaySpeedLabelConfig() const { return replaySpeedLabelConfig; }
        UITextConfig getReplaySpeedDisplayConfig() const { return replaySpeedDisplayConfig; }
        UITextConfig getReplayPressTConfig() const { return replayPressTConfig; }
        UITextConfig getReplayInstructionsConfig() const { return replayInstructionsConfig; }
        UITextConfig getWorldTitleConfig() const { return worldTitleConfig; }
        UITextConfig getStarIconConfig() const { return starIconConfig; }
        UITextConfig getStarCountConfig() const { return starCountConfig; }
        UITextConfig getModeTextConfig() const { return modeTextConfig; }
        UITextConfig getPressEConfig() const { return pressEConfig; }
        UITextConfig getControlsTextConfig() const { return controlsTextConfig; }
        
        // Ready画面UI要素の設定を取得
        UITextConfig getReadyTextConfig() const { return readyTextConfig; }
        UITextConfig getPlaySpeedLabelConfig() const { return playSpeedLabelConfig; }
        UISelectableConfig getSpeedOptionsConfig() const { return speedOptionsConfig; }
        UITextConfig getReadyPressTConfig() const { return readyPressTConfig; }
        UITextConfig getTpsFpsLabelConfig() const { return tpsFpsLabelConfig; }
        UISelectableConfig getTpsOptionConfig() const { return tpsOptionConfig; }
        UISelectableConfig getFpsOptionConfig() const { return fpsOptionConfig; }
        UITextConfig getReadyPressFConfig() const { return readyPressFConfig; }
        UITextConfig getConfirmConfig() const { return confirmConfig; }
        
        // Stage Clear UI
        UITextConfig getStageClearCompletedTextConfig() const { return stageClearCompletedTextConfig; }
        UITextConfig getStageClearClearTextConfig() const { return stageClearClearTextConfig; }
        UITextConfig getStageClearClearTimeConfig() const { return stageClearClearTimeConfig; }
        UITextConfig getStageClearReturnFieldConfig() const { return stageClearReturnFieldConfig; }
        UITextConfig getStageClearRetryConfig() const { return stageClearRetryConfig; }
        UISelectableConfig getStageClearStarsConfig() const { return stageClearStarsConfig; }
        
        // Game Over UI
        UITextConfig getGameOverTextConfig() const { return gameOverTextConfig; }
        UITextConfig getGameOverReturnFieldConfig() const { return gameOverReturnFieldConfig; }
        UITextConfig getGameOverRetryConfig() const { return gameOverRetryConfig; }
        
        // Mode Selection UI
        UITextConfig getModeSelectionTitleConfig() const { return modeSelectionTitleConfig; }
        UISelectableConfig getModeSelectionNormalTextConfig() const { return modeSelectionNormalTextConfig; }
        UISelectableConfig getModeSelectionTimeAttackTextConfig() const { return modeSelectionTimeAttackTextConfig; }
        UITextConfig getModeSelectionPressTConfig() const { return modeSelectionPressTConfig; }
        UITextConfig getModeSelectionConfirmConfig() const { return modeSelectionConfirmConfig; }
        
        // Tutorial UI
        UITextConfig getTutorialStepTextConfig() const { return tutorialStepTextConfig; }
        UITextConfig getTutorialMessageConfig() const { return tutorialMessageConfig; }
        UITextConfig getTutorialPressEnterConfig() const { return tutorialPressEnterConfig; }
        
        // Tutorial Step-specific UI
        UITextConfig getTutorialStep5ExplainText1Config() const { return tutorialStep5ExplainText1Config; }
        UITextConfig getTutorialStep9ExplainText1Config() const { return tutorialStep9ExplainText1Config; }
        UITextConfig getTutorialStep9ExplainText2Config() const { return tutorialStep9ExplainText2Config; }
        UITextConfig getTutorialStep9ExplainText3Config() const { return tutorialStep9ExplainText3Config; }
        UITextConfig getTutorialStep9ExplainText4Config() const { return tutorialStep9ExplainText4Config; }
        UITextConfig getTutorialStep10ExplainText1Config() const { return tutorialStep10ExplainText1Config; }
        UITextConfig getTutorialStep10ExplainText2Config() const { return tutorialStep10ExplainText2Config; }
        UITextConfig getTutorialStep10ExplainText3Config() const { return tutorialStep10ExplainText3Config; }
        
        // Unlock Confirm UI
        UITextConfig getUnlockConfirmTitleConfig() const { return unlockConfirmTitleConfig; }
        UITextConfig getUnlockConfirmRequiredStarsConfig() const { return unlockConfirmRequiredStarsConfig; }
        UITextConfig getUnlockConfirmUnlockButtonConfig() const { return unlockConfirmUnlockButtonConfig; }
        UITextConfig getUnlockConfirmCancelButtonConfig() const { return unlockConfirmCancelButtonConfig; }
        
        // Star Insufficient UI
        UITextConfig getStarInsufficientTitleConfig() const { return starInsufficientTitleConfig; }
        UITextConfig getStarInsufficientRequiredStarsConfig() const { return starInsufficientRequiredStarsConfig; }
        UITextConfig getStarInsufficientCollectStarsConfig() const { return starInsufficientCollectStarsConfig; }
        UITextConfig getStarInsufficientOkButtonConfig() const { return starInsufficientOkButtonConfig; }
        
        // Ending UI
        UITextConfig getEndingThankYouConfig() const { return endingThankYouConfig; }
        UITextConfig getEndingCongratulationsConfig() const { return endingCongratulationsConfig; }
        UITextConfig getEndingSeeYouAgainConfig() const { return endingSeeYouAgainConfig; }
        UITextConfig getEndingSkipConfig() const { return endingSkipConfig; }
        
        // Warp Tutorial UI
        UITextConfig getWarpTutorialTitleConfig() const { return warpTutorialTitleConfig; }
        UITextConfig getWarpTutorialDescription1Config() const { return warpTutorialDescription1Config; }
        UITextConfig getWarpTutorialDescription2Config() const { return warpTutorialDescription2Config; }
        UITextConfig getWarpTutorialEnterButtonConfig() const { return warpTutorialEnterButtonConfig; }
        
        // Stage 0 Tutorial UI
        UITextConfig getStage0TutorialLine1Config() const { return stage0TutorialLine1Config; }
        UITextConfig getStage0TutorialLine2Config() const { return stage0TutorialLine2Config; }
        UITextConfig getStage0TutorialLine3Config() const { return stage0TutorialLine3Config; }
        UITextConfig getStage0TutorialLine4Config() const { return stage0TutorialLine4Config; }
        UITextConfig getStage0TutorialLine5Config() const { return stage0TutorialLine5Config; }
        UITextConfig getStage0TutorialOkButtonConfig() const { return stage0TutorialOkButtonConfig; }
        
        // Easy Mode Explanation UI
        UITextConfig getEasyModeExplanationTitleConfig() const { return easyModeExplanationTitleConfig; }
        UITextConfig getEasyModeExplanationDescription1Config() const { return easyModeExplanationDescription1Config; }
        UITextConfig getEasyModeExplanationDescription2Config() const { return easyModeExplanationDescription2Config; }
        UITextConfig getEasyModeExplanationItem1Config() const { return easyModeExplanationItem1Config; }
        UITextConfig getEasyModeExplanationItem2Config() const { return easyModeExplanationItem2Config; }
        UITextConfig getEasyModeExplanationItem3Config() const { return easyModeExplanationItem3Config; }
        UITextConfig getEasyModeExplanationItem4aConfig() const { return easyModeExplanationItem4aConfig; }
        UITextConfig getEasyModeExplanationItem4bConfig() const { return easyModeExplanationItem4bConfig; }
        UITextConfig getEasyModeExplanationOkButtonConfig() const { return easyModeExplanationOkButtonConfig; }
        
        // Countdown UI
        UITextConfig getCountdownNumberConfig() const { return countdownNumberConfig; }
        
        // Stage Selection Assist UI
        UITextConfig getStageSelectionAssistTextConfig() const { return stageSelectionAssistTextConfig; }
        
        // Staff Roll UI
        UITextConfig getStaffRollSkipConfig() const { return staffRollSkipConfig; }
        UITextConfig getStaffRollTitleConfig() const { return staffRollTitleConfig; }
        UITextConfig getStaffRollRoleConfig() const { return staffRollRoleConfig; }
        UITextConfig getStaffRollNameConfig() const { return staffRollNameConfig; }
        float getStaffRollSpacing() const { return staffRollSpacing; }
        
        // Game UI
        UITimeDisplayConfig getGameUITimeDisplayConfig() const { return gameUITimeDisplayConfig; }
        UITextConfig getGameUITimeAttackDisplayConfig() const { return gameUITimeAttackDisplayConfig; }
        UITextConfig getGameUIBestTimeConfig() const { return gameUIBestTimeConfig; }
        UITextConfig getGameUIGoalDisplayConfig() const { return gameUIGoalDisplayConfig; }
        UITextConfig getGameUIGoalTime5sConfig() const { return gameUIGoalTime5sConfig; }
        UITextConfig getGameUIGoalTime10sConfig() const { return gameUIGoalTime10sConfig; }
        UITextConfig getGameUIGoalTime20sConfig() const { return gameUIGoalTime20sConfig; }
        UISelectableConfig getGameUIStarsConfig() const { return gameUIStarsConfig; }
        UISelectableConfig getGameUIHeartsConfig() const { return gameUIHeartsConfig; }
        UISkillConfig getGameUITimeStopSkillConfig() const { return gameUITimeStopSkillConfig; }
        UISkillConfig getGameUIDoubleJumpSkillConfig() const { return gameUIDoubleJumpSkillConfig; }
        UISkillConfig getGameUIHeartFeelSkillConfig() const { return gameUIHeartFeelSkillConfig; }
        UISkillConfig getGameUIFreeCameraSkillConfig() const { return gameUIFreeCameraSkillConfig; }
        UISkillConfig getGameUIBurstJumpSkillConfig() const { return gameUIBurstJumpSkillConfig; }
        
        // 位置を計算（ウィンドウサイズを考慮）
        glm::vec2 calculatePosition(const UIPosition& pos, int windowWidth, int windowHeight) const;
        
    private:
        UIConfigManager() = default;
        ~UIConfigManager() = default;
        UIConfigManager(const UIConfigManager&) = delete;
        UIConfigManager& operator=(const UIConfigManager&) = delete;
        
        void setDefaultValues();
        time_t getFileModificationTime(const std::string& filepath) const;
        
        std::string configFilePath;
        bool configLoaded = false;
        time_t lastFileModificationTime = 0;  // ファイル監視用
        
        // UI設定
        UITextConfig stageInfoConfig;
        UITextConfig speedDisplayConfig;
        UITextConfig pressTConfig;
        UITextConfig replayPauseMarkConfig;
        UITextConfig replayRewindMarkConfig;
        UITextConfig replayFastForwardMarkConfig;
        UITextConfig replaySpeedLabelConfig;
        UITextConfig replaySpeedDisplayConfig;
        UITextConfig replayPressTConfig;
        UITextConfig replayInstructionsConfig;
        UITextConfig worldTitleConfig;
        UITextConfig starIconConfig;
        UITextConfig starCountConfig;
        UITextConfig modeTextConfig;
        UITextConfig pressEConfig;
        UITextConfig controlsTextConfig;
        
        // Ready画面UI設定
        UITextConfig readyTextConfig;
        UITextConfig playSpeedLabelConfig;
        UISelectableConfig speedOptionsConfig;
        UITextConfig readyPressTConfig;
        UITextConfig tpsFpsLabelConfig;
        UISelectableConfig tpsOptionConfig;
        UISelectableConfig fpsOptionConfig;
        UITextConfig readyPressFConfig;
        UITextConfig confirmConfig;
        
        // Stage Clear UI設定
        UITextConfig stageClearCompletedTextConfig;
        UITextConfig stageClearClearTextConfig;
        UITextConfig stageClearClearTimeConfig;
        UITextConfig stageClearReturnFieldConfig;
        UITextConfig stageClearRetryConfig;
        UISelectableConfig stageClearStarsConfig;
        
        // Game Over UI設定
        UITextConfig gameOverTextConfig;
        UITextConfig gameOverReturnFieldConfig;
        UITextConfig gameOverRetryConfig;
        
        // Mode Selection UI設定
        UITextConfig modeSelectionTitleConfig;
        UISelectableConfig modeSelectionNormalTextConfig;
        UISelectableConfig modeSelectionTimeAttackTextConfig;
        UITextConfig modeSelectionPressTConfig;
        UITextConfig modeSelectionConfirmConfig;
        
        // Tutorial UI設定
        UITextConfig tutorialStepTextConfig;
        UITextConfig tutorialMessageConfig;
        UITextConfig tutorialPressEnterConfig;
        
        // Tutorial Step-specific UI設定
        UITextConfig tutorialStep5ExplainText1Config;
        UITextConfig tutorialStep9ExplainText1Config;
        UITextConfig tutorialStep9ExplainText2Config;
        UITextConfig tutorialStep9ExplainText3Config;
        UITextConfig tutorialStep9ExplainText4Config;
        UITextConfig tutorialStep10ExplainText1Config;
        UITextConfig tutorialStep10ExplainText2Config;
        UITextConfig tutorialStep10ExplainText3Config;
        
        // Unlock Confirm UI設定
        UITextConfig unlockConfirmTitleConfig;
        UITextConfig unlockConfirmRequiredStarsConfig;
        UITextConfig unlockConfirmUnlockButtonConfig;
        UITextConfig unlockConfirmCancelButtonConfig;
        
        // Star Insufficient UI設定
        UITextConfig starInsufficientTitleConfig;
        UITextConfig starInsufficientRequiredStarsConfig;
        UITextConfig starInsufficientCollectStarsConfig;
        UITextConfig starInsufficientOkButtonConfig;
        
        // Ending UI設定
        UITextConfig endingThankYouConfig;
        UITextConfig endingCongratulationsConfig;
        UITextConfig endingSeeYouAgainConfig;
        UITextConfig endingSkipConfig;
        
        // Warp Tutorial UI設定
        UITextConfig warpTutorialTitleConfig;
        UITextConfig warpTutorialDescription1Config;
        UITextConfig warpTutorialDescription2Config;
        UITextConfig warpTutorialEnterButtonConfig;
        
        // Stage 0 Tutorial UI設定
        UITextConfig stage0TutorialLine1Config;
        UITextConfig stage0TutorialLine2Config;
        UITextConfig stage0TutorialLine3Config;
        UITextConfig stage0TutorialLine4Config;
        UITextConfig stage0TutorialLine5Config;
        UITextConfig stage0TutorialOkButtonConfig;
        
        // Easy Mode Explanation UI設定
        UITextConfig easyModeExplanationTitleConfig;
        UITextConfig easyModeExplanationDescription1Config;
        UITextConfig easyModeExplanationDescription2Config;
        UITextConfig easyModeExplanationItem1Config;
        UITextConfig easyModeExplanationItem2Config;
        UITextConfig easyModeExplanationItem3Config;
        UITextConfig easyModeExplanationItem4aConfig;
        UITextConfig easyModeExplanationItem4bConfig;
        UITextConfig easyModeExplanationOkButtonConfig;
        
        // Countdown UI設定
        UITextConfig countdownNumberConfig;
        
        // Stage Selection Assist UI設定
        UITextConfig stageSelectionAssistTextConfig;
        
        // Staff Roll UI設定
        UITextConfig staffRollSkipConfig;
        UITextConfig staffRollTitleConfig;
        UITextConfig staffRollRoleConfig;
        UITextConfig staffRollNameConfig;
        float staffRollSpacing = 100.0f;
        
        // Game UI設定
        UITimeDisplayConfig gameUITimeDisplayConfig;
        UITextConfig gameUITimeAttackDisplayConfig;
        UITextConfig gameUIBestTimeConfig;
        UITextConfig gameUIGoalDisplayConfig;
        UITextConfig gameUIGoalTime5sConfig;
        UITextConfig gameUIGoalTime10sConfig;
        UITextConfig gameUIGoalTime20sConfig;
        UISelectableConfig gameUIStarsConfig;
        UISelectableConfig gameUIHeartsConfig;
        UISkillConfig gameUITimeStopSkillConfig;
        UISkillConfig gameUIDoubleJumpSkillConfig;
        UISkillConfig gameUIHeartFeelSkillConfig;
        UISkillConfig gameUIFreeCameraSkillConfig;
        UISkillConfig gameUIBurstJumpSkillConfig;
    };
}

