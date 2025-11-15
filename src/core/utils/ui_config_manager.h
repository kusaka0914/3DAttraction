/**
 * @file ui_config_manager.h
 * @brief UI設定マネージャー
 * @details JSONファイルからUI設定を読み込み、管理します。
 */
#pragma once

#include <string>
#include <glm/glm.hpp>
#include <map>
#include <type_traits>
#include <nlohmann/json.hpp>
#include "../error_handler.h"

namespace UIConfig {
    /**
     * @brief UI位置の構造体
     * @details 画面中央からの相対位置または画面左上基準の絶対位置を保持します。
     */
    struct UIPosition {
        float offsetX = 0.0f;
        float offsetY = 0.0f;
        float absoluteX = 0.0f;
        float absoluteY = 0.0f;
        bool useRelative = true;
    };
    
    /**
     * @brief UIテキスト設定の構造体
     * @details テキストUI要素の位置、色、スケールを保持します。
     */
    struct UITextConfig {
        UIPosition position;
        glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f);
        glm::vec3 completedColor = glm::vec3(1.0f, 1.0f, 1.0f);
        glm::vec3 activeColor = glm::vec3(1.0f, 1.0f, 1.0f);
        float scale = 1.0f;
    };
    
    /**
     * @brief UI選択可能要素設定の構造体
     * @details 選択可能なUI要素の位置、色、スケール、間隔を保持します。
     */
    struct UISelectableConfig {
        UIPosition position;
        glm::vec3 selectedColor = glm::vec3(1.0f, 0.8f, 0.2f);
        glm::vec3 unselectedColor = glm::vec3(0.5f, 0.5f, 0.5f);
        float scale = 1.0f;
        float spacing = 0.0f;
    };
    
    /**
     * @brief UI時間表示設定の構造体
     * @details 時間表示UI要素の位置、色、スケールを保持します。
     */
    struct UITimeDisplayConfig {
        UIPosition position;
        glm::vec3 normalColor = glm::vec3(1.0f, 1.0f, 1.0f);
        glm::vec3 warningColor = glm::vec3(1.0f, 0.5f, 0.0f);
        float scale = 1.0f;
    };
    
    /**
     * @brief UIスキル設定の構造体
     * @details スキルUI要素の位置、色、スケール、オフセットを保持します。
     */
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
        UIPosition activePosition;
        float activeScale = 1.0f;
    };
    
    /**
     * @brief UI設定マネージャー
     * @details UI要素の設定をJSONファイルから読み込み、管理します。
     */
    class UIConfigManager {
    public:
        /**
         * @brief シングルトンインスタンスを取得する
         * @return UIConfigManagerの参照
         */
        static UIConfigManager& getInstance();
        
        /**
         * @brief 設定を読み込む
         * @param filepath 設定ファイルのパス
         * @return 読み込み成功時true
         */
        bool loadConfig(const std::string& filepath = "assets/config/ui_config.json");
        
        /**
         * @brief 設定を再読み込みする
         * @details 設定ファイルを再読み込みします。
         */
        void reloadConfig();
        
        /**
         * @brief ファイル変更をチェックして再読み込みする
         * @details 設定ファイルが変更されていた場合、自動的に再読み込みします。
         * 
         * @return 再読み込みが発生した場合true
         */
        bool checkAndReloadConfig();
        
        // UI要素の設定を取得
        UITextConfig getStageInfoConfig() const { return stageInfoConfig; }
        UITextConfig getSpeedDisplayConfig() const { return speedDisplayConfig; }
        UITextConfig getPressTConfig() const { return pressTConfig; }
        UITextConfig getReplayPauseMarkConfig() const { return replayPauseMarkConfig; }
        UITextConfig getReplayRewindMarkConfig() const { return replayRewindMarkConfig; }
        UITextConfig getReplayFastForwardMarkConfig() const { return replayFastForwardMarkConfig; }
        UITextConfig getReplayRewindMarkAlwaysConfig() const { return replayRewindMarkAlwaysConfig; }
        UITextConfig getReplayPauseMarkAlwaysConfig() const { return replayPauseMarkAlwaysConfig; }
        UITextConfig getReplayFastForwardMarkAlwaysConfig() const { return replayFastForwardMarkAlwaysConfig; }
        UITextConfig getReplayPressAConfig() const { return replayPressAConfig; }
        UITextConfig getReplayPressSpaceConfig() const { return replayPressSpaceConfig; }
        UITextConfig getReplayPressDConfig() const { return replayPressDConfig; }
        UITextConfig getReplaySpeedLabelConfig() const { return replaySpeedLabelConfig; }
        UITextConfig getReplaySpeedDisplayConfig() const { return replaySpeedDisplayConfig; }
        UITextConfig getReplayPressTConfig() const { return replayPressTConfig; }
        UITextConfig getReplayInstructionsConfig() const { return replayInstructionsConfig; }
        UITextConfig getWorldTitleConfig() const { return worldTitleConfig; }
        UITextConfig getStarIconConfig() const { return starIconConfig; }
        UITextConfig getStarCountConfig() const { return starCountConfig; }
        UITextConfig getNormalModeTextConfig() const { return normalModeTextConfig; }
        UITextConfig getEasyModeTextConfig() const { return easyModeTextConfig; }
        UITextConfig getPressEConfig() const { return pressEConfig; }
        UITextConfig getControlsTextConfig() const { return controlsTextConfig; }
        
        // タイトル画面UI要素の設定を取得
        UITextConfig getTitleLogoConfig() const { return titleLogoConfig; }
        UITextConfig getTitleStartButtonConfig() const { return titleStartButtonConfig; }
        
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
        
        // Time Attack Clear UI
        UITextConfig getTimeAttackClearCompletedTextConfig() const { return timeAttackClearCompletedTextConfig; }
        UITextConfig getTimeAttackClearClearTextConfig() const { return timeAttackClearClearTextConfig; }
        UITextConfig getTimeAttackClearClearTimeConfig() const { return timeAttackClearClearTimeConfig; }
        UITextConfig getTimeAttackClearBestTimeConfig() const { return timeAttackClearBestTimeConfig; }
        UITextConfig getTimeAttackClearNewRecordConfig() const { return timeAttackClearNewRecordConfig; }
        UITextConfig getTimeAttackClearReturnFieldConfig() const { return timeAttackClearReturnFieldConfig; }
        UITextConfig getTimeAttackClearRetryConfig() const { return timeAttackClearRetryConfig; }
        UITextConfig getTimeAttackClearReplayConfig() const { return timeAttackClearReplayConfig; }
        
        // Game Over UI
        UITextConfig getGameOverTextConfig() const { return gameOverTextConfig; }
        UITextConfig getGameOverReturnFieldConfig() const { return gameOverReturnFieldConfig; }
        UITextConfig getGameOverRetryConfig() const { return gameOverRetryConfig; }
        
        // Mode Selection UI
        UITextConfig getModeSelectionTitleConfig() const { return modeSelectionTitleConfig; }
        UISelectableConfig getModeSelectionNormalTextConfig() const { return modeSelectionNormalTextConfig; }
        UISelectableConfig getModeSelectionEasyTextConfig() const { return modeSelectionEasyTextConfig; }
        UISelectableConfig getModeSelectionTimeAttackTextConfig() const { return modeSelectionTimeAttackTextConfig; }
        UISelectableConfig getModeSelectionSecretStarTextConfig() const { return modeSelectionSecretStarTextConfig; }
        UITextConfig getModeSelectionPressTConfig() const { return modeSelectionPressTConfig; }
        UITextConfig getModeSelectionConfirmConfig() const { return modeSelectionConfirmConfig; }
        
        // SECRET STAR Explanation UI
        UITextConfig getSecretStarExplanationLine1Config() const { return secretStarExplanationLine1Config; }
        UITextConfig getSecretStarExplanationLine2Config() const { return secretStarExplanationLine2Config; }
        UITextConfig getSecretStarExplanationLine3Config() const { return secretStarExplanationLine3Config; }
        UITextConfig getSecretStarExplanationLine3bConfig() const { return secretStarExplanationLine3bConfig; }
        UITextConfig getSecretStarExplanationLine4Config() const { return secretStarExplanationLine4Config; }
        UITextConfig getSecretStarExplanationLine4bConfig() const { return secretStarExplanationLine4bConfig; }
        UITextConfig getSecretStarExplanationLine4cConfig() const { return secretStarExplanationLine4cConfig; }
        UITextConfig getSecretStarExplanationLine5Config() const { return secretStarExplanationLine5Config; }
        UITextConfig getSecretStarExplanationLine5bConfig() const { return secretStarExplanationLine5bConfig; }
        UITextConfig getSecretStarExplanationOkButtonConfig() const { return secretStarExplanationOkButtonConfig; }
        
        // SECRET STAR Selection UI
        UITextConfig getSecretStarSelectionTitleConfig() const { return secretStarSelectionTitleConfig; }
        UIPosition getSecretStarSelectionStar1Config() const { return secretStarSelectionStar1Config; }
        UIPosition getSecretStarSelectionStar2Config() const { return secretStarSelectionStar2Config; }
        UIPosition getSecretStarSelectionStar3Config() const { return secretStarSelectionStar3Config; }
        UISelectableConfig getSecretStarSelectionName1Config() const { return secretStarSelectionName1Config; }
        UISelectableConfig getSecretStarSelectionName2Config() const { return secretStarSelectionName2Config; }
        UISelectableConfig getSecretStarSelectionName3Config() const { return secretStarSelectionName3Config; }
        UITextConfig getSecretStarSelectionPressTConfig() const { return secretStarSelectionPressTConfig; }
        UITextConfig getSecretStarSelectionConfirmConfig() const { return secretStarSelectionConfirmConfig; }
        
        // Tutorial UI
        UITextConfig getTutorialStepTextConfig() const { return tutorialStepTextConfig; }
        UITextConfig getTutorialMessageConfig() const { return tutorialMessageConfig; }
        UITextConfig getTutorialMessageConfigForStep(int step) const;
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
        UITextConfig getStageSelectionEscKeyInfoConfig() const { return stageSelectionEscKeyInfoConfig; }
        
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
        UIPosition getGameUITimeAttackSpeedDisplayPosition() const { return gameUITimeAttackSpeedDisplayPosition; }
        UIPosition getGameUITimeAttackPressTPosition() const { return gameUITimeAttackPressTPosition; }
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
        
        // 動的UI要素登録システム
        // JSONパスを指定して設定を取得（例: "gameUI.timeDisplay", "readyScreen.title"）
        template<typename T>
        T getUIConfig(const std::string& jsonPath) const {
            nlohmann::json jsonValue = getJsonValue(jsonPath);
            
            if (jsonValue.is_null()) {
                // パスが見つからない場合はデフォルト値を返す
                printf("UI Config Warning: Path '%s' not found, using default values\n", jsonPath.c_str());
                T defaultConfig;
                return defaultConfig;
            }
            
            // 型に応じて解析
            if constexpr (std::is_same_v<T, UITextConfig>) {
                return parseUITextConfig(jsonValue);
            } else if constexpr (std::is_same_v<T, UISelectableConfig>) {
                return parseUISelectableConfig(jsonValue);
            } else if constexpr (std::is_same_v<T, UITimeDisplayConfig>) {
                return parseUITimeDisplayConfig(jsonValue);
            } else if constexpr (std::is_same_v<T, UISkillConfig>) {
                return parseUISkillConfig(jsonValue);
            } else {
                // 未知の型
                ErrorHandler::logErrorFormat("UI Config Error: Unsupported type for path '%s'", jsonPath.c_str());
                T defaultConfig;
                return defaultConfig;
            }
        }
        
    private:
        UIConfigManager() = default;
        ~UIConfigManager() = default;
        UIConfigManager(const UIConfigManager&) = delete;
        UIConfigManager& operator=(const UIConfigManager&) = delete;
        
        void setDefaultValues();
        time_t getFileModificationTime(const std::string& filepath) const;
        
        std::string configFilePath;
        bool configLoaded = false;
        time_t lastFileModificationTime = 0;
        nlohmann::json cachedJsonData;
        
        // JSONから構造体への変換ヘルパー関数
        UITextConfig parseUITextConfig(const nlohmann::json& json) const;
        UISelectableConfig parseUISelectableConfig(const nlohmann::json& json) const;
        UITimeDisplayConfig parseUITimeDisplayConfig(const nlohmann::json& json) const;
        UISkillConfig parseUISkillConfig(const nlohmann::json& json) const;
        UIPosition parseUIPosition(const nlohmann::json& json) const;
        
        // JSONパスから値を取得（内部用）
        nlohmann::json getJsonValue(const std::string& jsonPath) const;
        
        // UI設定
        UITextConfig stageInfoConfig;
        UITextConfig speedDisplayConfig;
        UITextConfig pressTConfig;
        UITextConfig replayPauseMarkConfig;
        UITextConfig replayRewindMarkConfig;
        UITextConfig replayFastForwardMarkConfig;
        UITextConfig replayRewindMarkAlwaysConfig;
        UITextConfig replayPauseMarkAlwaysConfig;
        UITextConfig replayFastForwardMarkAlwaysConfig;
        UITextConfig replayPressAConfig;
        UITextConfig replayPressSpaceConfig;
        UITextConfig replayPressDConfig;
        UITextConfig replaySpeedLabelConfig;
        UITextConfig replaySpeedDisplayConfig;
        UITextConfig replayPressTConfig;
        UITextConfig replayInstructionsConfig;
        UITextConfig titleLogoConfig;
        UITextConfig titleStartButtonConfig;
        UITextConfig worldTitleConfig;
        UITextConfig starIconConfig;
        UITextConfig starCountConfig;
        UITextConfig normalModeTextConfig;
        UITextConfig easyModeTextConfig;
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
        
        // Time Attack Clear UI設定
        UITextConfig timeAttackClearCompletedTextConfig;
        UITextConfig timeAttackClearClearTextConfig;
        UITextConfig timeAttackClearClearTimeConfig;
        UITextConfig timeAttackClearBestTimeConfig;
        UITextConfig timeAttackClearNewRecordConfig;
        UITextConfig timeAttackClearReturnFieldConfig;
        UITextConfig timeAttackClearRetryConfig;
        UITextConfig timeAttackClearReplayConfig;
        
        // Game Over UI設定
        UITextConfig gameOverTextConfig;
        UITextConfig gameOverReturnFieldConfig;
        UITextConfig gameOverRetryConfig;
        
        // Mode Selection UI設定
        UITextConfig modeSelectionTitleConfig;
        UISelectableConfig modeSelectionNormalTextConfig;
        UISelectableConfig modeSelectionEasyTextConfig;
        UISelectableConfig modeSelectionTimeAttackTextConfig;
        UISelectableConfig modeSelectionSecretStarTextConfig;
        UITextConfig modeSelectionPressTConfig;
        UITextConfig modeSelectionConfirmConfig;
        
        // SECRET STAR Explanation UI設定
        UITextConfig secretStarExplanationLine1Config;
        UITextConfig secretStarExplanationLine2Config;
        UITextConfig secretStarExplanationLine3Config;
        UITextConfig secretStarExplanationLine3bConfig;
        UITextConfig secretStarExplanationLine4Config;
        UITextConfig secretStarExplanationLine4bConfig;
        UITextConfig secretStarExplanationLine4cConfig;
        UITextConfig secretStarExplanationLine5Config;
        UITextConfig secretStarExplanationLine5bConfig;
        UITextConfig secretStarExplanationOkButtonConfig;
        
        // SECRET STAR Selection UI設定
        UITextConfig secretStarSelectionTitleConfig;
        UIPosition secretStarSelectionStar1Config;
        UIPosition secretStarSelectionStar2Config;
        UIPosition secretStarSelectionStar3Config;
        UISelectableConfig secretStarSelectionName1Config;
        UISelectableConfig secretStarSelectionName2Config;
        UISelectableConfig secretStarSelectionName3Config;
        UITextConfig secretStarSelectionPressTConfig;
        UITextConfig secretStarSelectionConfirmConfig;
        
        // Tutorial UI設定
        UITextConfig tutorialStepTextConfig;
        UITextConfig tutorialMessageConfig;
        UITextConfig tutorialMessageConfigs[11]; // step0-10用の個別設定
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
        UITextConfig stageSelectionEscKeyInfoConfig;
        
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
        UIPosition gameUITimeAttackSpeedDisplayPosition;
        UIPosition gameUITimeAttackPressTPosition;
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

