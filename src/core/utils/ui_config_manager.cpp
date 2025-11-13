#include "ui_config_manager.h"
#include <fstream>
#include <iostream>
#include <vector>
#include <nlohmann/json.hpp>
#include <ctime>
#ifdef _WIN32
    #include <sys/stat.h>
    #include <io.h>
#else
    #include <sys/stat.h>
    #include <unistd.h>
#endif

namespace UIConfig {
    UIConfigManager& UIConfigManager::getInstance() {
        static UIConfigManager instance;
        return instance;
    }
    
    void UIConfigManager::setDefaultValues() {
        // ステージ情報（STAGE X / TUTORIAL）
        stageInfoConfig.position.useRelative = false;
        stageInfoConfig.position.absoluteX = 30.0f;
        stageInfoConfig.position.absoluteY = 30.0f;
        stageInfoConfig.color = glm::vec3(1.0f, 1.0f, 0.0f);
        stageInfoConfig.scale = 2.0f;
        
        // 速度倍率表示（3xなど）
        speedDisplayConfig.position.useRelative = false;
        speedDisplayConfig.position.absoluteX = 880.0f;
        speedDisplayConfig.position.absoluteY = 25.0f;
        speedDisplayConfig.color = glm::vec3(1.0f, 1.0f, 1.0f);
        speedDisplayConfig.scale = 2.0f;
        
        // PRESS T表示（通常モード）
        pressTConfig.position.useRelative = false;
        pressTConfig.position.absoluteX = 870.0f;
        pressTConfig.position.absoluteY = 65.0f;
        pressTConfig.color = glm::vec3(1.0f, 1.0f, 1.0f);
        pressTConfig.scale = 1.0f;
        
        // リプレイポーズマーク（||）
        replayPauseMarkConfig.position.useRelative = true;
        replayPauseMarkConfig.position.offsetX = -200.0f;
        replayPauseMarkConfig.position.offsetY = -280.0f;
        replayPauseMarkConfig.color = glm::vec3(1.0f, 0.8f, 0.2f);
        replayPauseMarkConfig.scale = 5.0f;
        
        // リプレイ巻き戻しマーク（<<）
        replayRewindMarkConfig.position.useRelative = true;
        replayRewindMarkConfig.position.offsetX = -60.0f;
        replayRewindMarkConfig.position.offsetY = -80.0f;
        replayRewindMarkConfig.color = glm::vec3(1.0f, 0.8f, 0.2f);
        replayRewindMarkConfig.scale = 5.0f;
        
        // リプレイ早送りマーク（>>）
        replayFastForwardMarkConfig.position.useRelative = true;
        replayFastForwardMarkConfig.position.offsetX = -60.0f;
        replayFastForwardMarkConfig.position.offsetY = -80.0f;
        replayFastForwardMarkConfig.color = glm::vec3(1.0f, 0.8f, 0.2f);
        replayFastForwardMarkConfig.scale = 5.0f;
        
        // リプレイ速度ラベル（REPLAY SPEED）
        replaySpeedLabelConfig.position.useRelative = true;
        replaySpeedLabelConfig.position.offsetX = -80.0f;
        replaySpeedLabelConfig.position.offsetY = 60.0f;
        replaySpeedLabelConfig.color = glm::vec3(0.8f, 0.8f, 0.8f);
        replaySpeedLabelConfig.scale = 1.2f;
        
        // リプレイ速度表示（1.0xなど）
        replaySpeedDisplayConfig.position.useRelative = true;
        replaySpeedDisplayConfig.position.offsetX = -40.0f;
        replaySpeedDisplayConfig.position.offsetY = 100.0f;
        replaySpeedDisplayConfig.color = glm::vec3(0.8f, 0.8f, 0.8f);
        replaySpeedDisplayConfig.scale = 1.5f;
        
        // リプレイPRESS T表示
        replayPressTConfig.position.useRelative = true;
        replayPressTConfig.position.offsetX = -60.0f;
        replayPressTConfig.position.offsetY = 140.0f;
        replayPressTConfig.color = glm::vec3(0.8f, 0.8f, 0.8f);
        replayPressTConfig.scale = 1.0f;
        
        // リプレイ操作説明
        replayInstructionsConfig.position.useRelative = true;
        replayInstructionsConfig.position.offsetX = -300.0f;
        replayInstructionsConfig.position.offsetY = -50.0f;  // 画面下から50px上
        replayInstructionsConfig.color = glm::vec3(0.8f, 0.8f, 0.8f);
        replayInstructionsConfig.scale = 1.0f;
        
        // WORLD 1表示
        worldTitleConfig.position.useRelative = true;
        worldTitleConfig.position.offsetX = -50.0f;
        worldTitleConfig.position.offsetY = -690.0f;  // 画面上部（30px下）
        worldTitleConfig.color = glm::vec3(1.0f, 1.0f, 0.0f);
        worldTitleConfig.scale = 1.5f;
        
        // 星数表示
        starCountConfig.position.useRelative = false;
        starCountConfig.position.absoluteX = 72.0f;
        starCountConfig.position.absoluteY = 27.0f;
        starCountConfig.color = glm::vec3(1.0f, 1.0f, 0.0f);
        starCountConfig.scale = 1.5f;
        
        // EASY/NORMAL表示
        modeTextConfig.position.useRelative = false;
        modeTextConfig.position.absoluteX = 140.0f;
        modeTextConfig.position.absoluteY = 20.0f;
        modeTextConfig.color = glm::vec3(1.0f, 1.0f, 1.0f);
        modeTextConfig.scale = 1.5f;
        
        // PRESS E表示
        pressEConfig.position.useRelative = false;
        pressEConfig.position.absoluteX = 155.0f;
        pressEConfig.position.absoluteY = 50.0f;
        pressEConfig.color = glm::vec3(1.0f, 1.0f, 1.0f);
        pressEConfig.scale = 0.8f;
        
        // 星アイコン（ステージ選択フィールド）
        starIconConfig.position.useRelative = false;
        starIconConfig.position.absoluteX = 70.0f;
        starIconConfig.position.absoluteY = 70.0f;
        starIconConfig.color = glm::vec3(1.0f, 1.0f, 0.0f);
        starIconConfig.scale = 3.0f;
        
        // 操作説明テキスト
        controlsTextConfig.position.useRelative = false;
        controlsTextConfig.position.absoluteX = 10.0f;
        controlsTextConfig.position.offsetY = -30.0f;  // 画面下から30px上（相対）
        controlsTextConfig.color = glm::vec3(0.8f, 0.8f, 0.8f);
        controlsTextConfig.scale = 1.0f;
        
        // Ready画面UI設定
        readyTextConfig.position.useRelative = true;
        readyTextConfig.position.offsetX = -230.0f;
        readyTextConfig.position.offsetY = -300.0f;
        readyTextConfig.color = glm::vec3(1.0f, 1.0f, 1.0f);
        readyTextConfig.scale = 5.0f;
        
        playSpeedLabelConfig.position.useRelative = true;
        playSpeedLabelConfig.position.offsetX = -400.0f;
        playSpeedLabelConfig.position.offsetY = -70.0f;
        playSpeedLabelConfig.color = glm::vec3(1.0f, 1.0f, 1.0f);
        playSpeedLabelConfig.scale = 1.5f;
        
        speedOptionsConfig.position.useRelative = true;
        speedOptionsConfig.position.offsetX = -450.0f;
        speedOptionsConfig.position.offsetY = 0.0f;
        speedOptionsConfig.spacing = 150.0f;
        speedOptionsConfig.selectedColor = glm::vec3(1.0f, 0.8f, 0.2f);
        speedOptionsConfig.unselectedColor = glm::vec3(0.5f, 0.5f, 0.5f);
        speedOptionsConfig.scale = 2.0f;
        
        readyPressTConfig.position.useRelative = true;
        readyPressTConfig.position.offsetX = -350.0f;
        readyPressTConfig.position.offsetY = 100.0f;
        readyPressTConfig.color = glm::vec3(1.0f, 0.8f, 0.2f);
        readyPressTConfig.scale = 1.2f;
        
        tpsFpsLabelConfig.position.useRelative = true;
        tpsFpsLabelConfig.position.offsetX = 190.0f;
        tpsFpsLabelConfig.position.offsetY = -70.0f;
        tpsFpsLabelConfig.color = glm::vec3(1.0f, 1.0f, 1.0f);
        tpsFpsLabelConfig.scale = 1.5f;
        
        tpsOptionConfig.position.useRelative = true;
        tpsOptionConfig.position.offsetX = 130.0f;
        tpsOptionConfig.position.offsetY = 0.0f;
        tpsOptionConfig.selectedColor = glm::vec3(1.0f, 0.8f, 0.2f);
        tpsOptionConfig.unselectedColor = glm::vec3(0.5f, 0.5f, 0.5f);
        tpsOptionConfig.scale = 2.0f;
        
        fpsOptionConfig.position.useRelative = true;
        fpsOptionConfig.position.offsetX = 320.0f;
        fpsOptionConfig.position.offsetY = 0.0f;
        fpsOptionConfig.selectedColor = glm::vec3(1.0f, 0.8f, 0.2f);
        fpsOptionConfig.unselectedColor = glm::vec3(0.5f, 0.5f, 0.5f);
        fpsOptionConfig.scale = 2.0f;
        
        readyPressFConfig.position.useRelative = true;
        readyPressFConfig.position.offsetX = 210.0f;
        readyPressFConfig.position.offsetY = 100.0f;
        readyPressFConfig.color = glm::vec3(1.0f, 0.8f, 0.2f);
        readyPressFConfig.scale = 1.2f;
        
        confirmConfig.position.useRelative = true;
        confirmConfig.position.offsetX = -150.0f;
        confirmConfig.position.offsetY = 250.0f;
        confirmConfig.color = glm::vec3(0.2f, 0.8f, 0.2f);
        confirmConfig.scale = 1.2f;
        
        // Stage Clear UI設定
        stageClearCompletedTextConfig.position.useRelative = true;
        stageClearCompletedTextConfig.position.offsetX = -300.0f;
        stageClearCompletedTextConfig.position.offsetY = -80.0f;
        stageClearCompletedTextConfig.color = glm::vec3(0.2f, 0.8f, 0.2f);
        stageClearCompletedTextConfig.scale = 2.5f;
        
        stageClearClearTextConfig.position.useRelative = true;
        stageClearClearTextConfig.position.offsetX = -200.0f;
        stageClearClearTextConfig.position.offsetY = -80.0f;
        stageClearClearTextConfig.color = glm::vec3(0.2f, 0.8f, 0.2f);
        stageClearClearTextConfig.scale = 2.5f;
        
        stageClearClearTimeConfig.position.useRelative = true;
        stageClearClearTimeConfig.position.offsetX = -400.0f;
        stageClearClearTimeConfig.position.offsetY = 350.0f;
        stageClearClearTimeConfig.color = glm::vec3(1.0f, 1.0f, 1.0f);
        stageClearClearTimeConfig.scale = 1.8f;
        
        stageClearReturnFieldConfig.position.useRelative = true;
        stageClearReturnFieldConfig.position.offsetX = -550.0f;
        stageClearReturnFieldConfig.position.offsetY = 500.0f;
        stageClearReturnFieldConfig.color = glm::vec3(0.2f, 0.8f, 0.2f);
        stageClearReturnFieldConfig.scale = 1.0f;
        
        stageClearRetryConfig.position.useRelative = true;
        stageClearRetryConfig.position.offsetX = 150.0f;
        stageClearRetryConfig.position.offsetY = 500.0f;
        stageClearRetryConfig.color = glm::vec3(0.8f, 0.8f, 0.8f);
        stageClearRetryConfig.scale = 1.0f;
        
        stageClearStarsConfig.position.useRelative = true;
        stageClearStarsConfig.position.offsetX = 750.0f;
        stageClearStarsConfig.position.offsetY = 720.0f;
        stageClearStarsConfig.spacing = 600.0f;
        stageClearStarsConfig.selectedColor = glm::vec3(1.0f, 1.0f, 0.0f);
        stageClearStarsConfig.unselectedColor = glm::vec3(0.5f, 0.5f, 0.5f);
        stageClearStarsConfig.scale = 10.0f;
        
        // Game Over UI設定
        gameOverTextConfig.position.useRelative = true;
        gameOverTextConfig.position.offsetX = -300.0f;
        gameOverTextConfig.position.offsetY = -100.0f;
        gameOverTextConfig.color = glm::vec3(1.0f, 0.2f, 0.2f);
        gameOverTextConfig.scale = 3.0f;
        
        gameOverReturnFieldConfig.position.useRelative = true;
        gameOverReturnFieldConfig.position.offsetX = -300.0f;
        gameOverReturnFieldConfig.position.offsetY = 50.0f;
        gameOverReturnFieldConfig.color = glm::vec3(0.2f, 0.8f, 0.2f);
        gameOverReturnFieldConfig.scale = 1.5f;
        
        gameOverRetryConfig.position.useRelative = true;
        gameOverRetryConfig.position.offsetX = -150.0f;
        gameOverRetryConfig.position.offsetY = 150.0f;
        gameOverRetryConfig.color = glm::vec3(0.8f, 0.8f, 0.8f);
        gameOverRetryConfig.scale = 1.5f;
        
        // Mode Selection UI設定
        modeSelectionTitleConfig.position.useRelative = true;
        modeSelectionTitleConfig.position.offsetX = -250.0f;
        modeSelectionTitleConfig.position.offsetY = -200.0f;
        modeSelectionTitleConfig.color = glm::vec3(1.0f, 1.0f, 0.0f);
        modeSelectionTitleConfig.scale = 2.0f;
        
        modeSelectionNormalTextConfig.position.useRelative = true;
        modeSelectionNormalTextConfig.position.offsetX = -200.0f;
        modeSelectionNormalTextConfig.position.offsetY = -50.0f;
        modeSelectionNormalTextConfig.selectedColor = glm::vec3(1.0f, 0.8f, 0.2f);
        modeSelectionNormalTextConfig.unselectedColor = glm::vec3(0.5f, 0.5f, 0.5f);
        modeSelectionNormalTextConfig.scale = 2.0f;
        
        modeSelectionTimeAttackTextConfig.position.useRelative = true;
        modeSelectionTimeAttackTextConfig.position.offsetX = 20.0f;
        modeSelectionTimeAttackTextConfig.position.offsetY = -50.0f;
        modeSelectionTimeAttackTextConfig.selectedColor = glm::vec3(1.0f, 0.8f, 0.2f);
        modeSelectionTimeAttackTextConfig.unselectedColor = glm::vec3(0.5f, 0.5f, 0.5f);
        modeSelectionTimeAttackTextConfig.scale = 2.0f;
        
        modeSelectionPressTConfig.position.useRelative = true;
        modeSelectionPressTConfig.position.offsetX = -80.0f;
        modeSelectionPressTConfig.position.offsetY = 50.0f;
        modeSelectionPressTConfig.color = glm::vec3(1.0f, 0.8f, 0.2f);
        modeSelectionPressTConfig.scale = 1.2f;
        
        modeSelectionConfirmConfig.position.useRelative = true;
        modeSelectionConfirmConfig.position.offsetX = -200.0f;
        modeSelectionConfirmConfig.position.offsetY = 150.0f;
        modeSelectionConfirmConfig.color = glm::vec3(0.2f, 0.8f, 0.2f);
        modeSelectionConfirmConfig.scale = 1.5f;
        
        // Tutorial UI設定
        tutorialStepTextConfig.position.useRelative = true;
        tutorialStepTextConfig.position.offsetX = -70.0f;
        tutorialStepTextConfig.position.offsetY = 90.0f;  // 以前のcenterY=500基準に合わせて調整 (500-50-360=90)
        tutorialStepTextConfig.color = glm::vec3(1.0f, 1.0f, 1.0f);
        tutorialStepTextConfig.scale = 1.2f;
        
        tutorialMessageConfig.position.useRelative = true;
        tutorialMessageConfig.position.offsetX = -200.0f;
        tutorialMessageConfig.position.offsetY = 140.0f;  // 以前のcenterY=500基準に合わせて調整 (500-360=140)
        tutorialMessageConfig.color = glm::vec3(1.0f, 1.0f, 1.0f);
        tutorialMessageConfig.completedColor = glm::vec3(0.2f, 0.8f, 0.2f);
        tutorialMessageConfig.scale = 1.5f;
        
        tutorialPressEnterConfig.position.useRelative = true;
        tutorialPressEnterConfig.position.offsetX = -150.0f;
        tutorialPressEnterConfig.position.offsetY = 190.0f;  // 以前のcenterY=500基準に合わせて調整 (500+50-360=190)
        tutorialPressEnterConfig.color = glm::vec3(0.2f, 0.8f, 0.2f);
        tutorialPressEnterConfig.scale = 1.2f;
        
        // Tutorial Step-specific UI設定
        // Step 5
        tutorialStep5ExplainText1Config.position.useRelative = true;
        tutorialStep5ExplainText1Config.position.offsetX = -50.0f;
        tutorialStep5ExplainText1Config.position.offsetY = -400.0f;
        tutorialStep5ExplainText1Config.color = glm::vec3(1.0f, 1.0f, 1.0f);
        tutorialStep5ExplainText1Config.scale = 1.2f;
        
        // Step 9
        tutorialStep9ExplainText1Config.position.useRelative = true;
        tutorialStep9ExplainText1Config.position.offsetX = -200.0f;
        tutorialStep9ExplainText1Config.position.offsetY = 50.0f;
        tutorialStep9ExplainText1Config.color = glm::vec3(1.0f, 1.0f, 1.0f);
        tutorialStep9ExplainText1Config.scale = 1.2f;
        
        tutorialStep9ExplainText2Config.position.useRelative = true;
        tutorialStep9ExplainText2Config.position.offsetX = -120.0f;
        tutorialStep9ExplainText2Config.position.offsetY = 90.0f;
        tutorialStep9ExplainText2Config.color = glm::vec3(1.0f, 1.0f, 1.0f);
        tutorialStep9ExplainText2Config.scale = 1.2f;
        
        tutorialStep9ExplainText3Config.position.useRelative = true;
        tutorialStep9ExplainText3Config.position.offsetX = -200.0f;
        tutorialStep9ExplainText3Config.position.offsetY = 130.0f;
        tutorialStep9ExplainText3Config.color = glm::vec3(1.0f, 1.0f, 1.0f);
        tutorialStep9ExplainText3Config.scale = 1.2f;
        
        tutorialStep9ExplainText4Config.position.useRelative = true;
        tutorialStep9ExplainText4Config.position.offsetX = -150.0f;
        tutorialStep9ExplainText4Config.position.offsetY = 170.0f;
        tutorialStep9ExplainText4Config.color = glm::vec3(1.0f, 1.0f, 1.0f);
        tutorialStep9ExplainText4Config.scale = 1.2f;
        
        // Step 10
        tutorialStep10ExplainText1Config.position.useRelative = true;
        tutorialStep10ExplainText1Config.position.offsetX = -190.0f;
        tutorialStep10ExplainText1Config.position.offsetY = 50.0f;
        tutorialStep10ExplainText1Config.color = glm::vec3(1.0f, 1.0f, 1.0f);
        tutorialStep10ExplainText1Config.scale = 1.2f;
        
        tutorialStep10ExplainText2Config.position.useRelative = true;
        tutorialStep10ExplainText2Config.position.offsetX = -300.0f;
        tutorialStep10ExplainText2Config.position.offsetY = 90.0f;
        tutorialStep10ExplainText2Config.color = glm::vec3(1.0f, 1.0f, 1.0f);
        tutorialStep10ExplainText2Config.scale = 1.2f;
        
        tutorialStep10ExplainText3Config.position.useRelative = true;
        tutorialStep10ExplainText3Config.position.offsetX = -90.0f;
        tutorialStep10ExplainText3Config.position.offsetY = 130.0f;
        tutorialStep10ExplainText3Config.color = glm::vec3(1.0f, 1.0f, 1.0f);
        tutorialStep10ExplainText3Config.scale = 1.2f;
        
        // Unlock Confirm UI設定
        unlockConfirmTitleConfig.position.useRelative = true;
        unlockConfirmTitleConfig.position.offsetX = -250.0f;
        unlockConfirmTitleConfig.position.offsetY = -200.0f;
        unlockConfirmTitleConfig.color = glm::vec3(1.0f, 1.0f, 0.0f);
        unlockConfirmTitleConfig.scale = 2.0f;
        
        unlockConfirmRequiredStarsConfig.position.useRelative = true;
        unlockConfirmRequiredStarsConfig.position.offsetX = -250.0f;
        unlockConfirmRequiredStarsConfig.position.offsetY = -50.0f;
        unlockConfirmRequiredStarsConfig.color = glm::vec3(1.0f, 0.8f, 0.2f);
        unlockConfirmRequiredStarsConfig.scale = 1.5f;
        
        unlockConfirmUnlockButtonConfig.position.useRelative = true;
        unlockConfirmUnlockButtonConfig.position.offsetX = -300.0f;
        unlockConfirmUnlockButtonConfig.position.offsetY = 100.0f;
        unlockConfirmUnlockButtonConfig.color = glm::vec3(0.2f, 0.8f, 0.2f);
        unlockConfirmUnlockButtonConfig.scale = 1.5f;
        
        unlockConfirmCancelButtonConfig.position.useRelative = true;
        unlockConfirmCancelButtonConfig.position.offsetX = 100.0f;
        unlockConfirmCancelButtonConfig.position.offsetY = 100.0f;
        unlockConfirmCancelButtonConfig.color = glm::vec3(0.8f, 0.2f, 0.2f);
        unlockConfirmCancelButtonConfig.scale = 1.5f;
        
        // Star Insufficient UI設定
        starInsufficientTitleConfig.position.useRelative = true;
        starInsufficientTitleConfig.position.offsetX = -350.0f;
        starInsufficientTitleConfig.position.offsetY = -200.0f;
        starInsufficientTitleConfig.color = glm::vec3(1.0f, 0.2f, 0.2f);
        starInsufficientTitleConfig.scale = 2.0f;
        
        starInsufficientRequiredStarsConfig.position.useRelative = true;
        starInsufficientRequiredStarsConfig.position.offsetX = -200.0f;
        starInsufficientRequiredStarsConfig.position.offsetY = -50.0f;
        starInsufficientRequiredStarsConfig.color = glm::vec3(1.0f, 0.8f, 0.2f);
        starInsufficientRequiredStarsConfig.scale = 1.5f;
        
        starInsufficientCollectStarsConfig.position.useRelative = true;
        starInsufficientCollectStarsConfig.position.offsetX = -400.0f;
        starInsufficientCollectStarsConfig.position.offsetY = 100.0f;
        starInsufficientCollectStarsConfig.color = glm::vec3(1.0f, 1.0f, 1.0f);
        starInsufficientCollectStarsConfig.scale = 1.5f;
        
        starInsufficientOkButtonConfig.position.useRelative = true;
        starInsufficientOkButtonConfig.position.offsetX = -100.0f;
        starInsufficientOkButtonConfig.position.offsetY = 200.0f;
        starInsufficientOkButtonConfig.color = glm::vec3(0.2f, 0.8f, 0.2f);
        starInsufficientOkButtonConfig.scale = 1.5f;
        
        // Ending UI設定
        endingThankYouConfig.position.useRelative = true;
        endingThankYouConfig.position.offsetX = -700.0f;
        endingThankYouConfig.position.offsetY = -400.0f;
        endingThankYouConfig.color = glm::vec3(1.0f, 1.0f, 0.0f);
        endingThankYouConfig.scale = 2.0f;
        
        endingCongratulationsConfig.position.useRelative = true;
        endingCongratulationsConfig.position.offsetX = -850.0f;
        endingCongratulationsConfig.position.offsetY = -300.0f;
        endingCongratulationsConfig.color = glm::vec3(1.0f, 1.0f, 0.0f);
        endingCongratulationsConfig.scale = 1.8f;
        
        endingSeeYouAgainConfig.position.useRelative = true;
        endingSeeYouAgainConfig.position.offsetX = -700.0f;
        endingSeeYouAgainConfig.position.offsetY = -200.0f;
        endingSeeYouAgainConfig.color = glm::vec3(1.0f, 1.0f, 0.0f);
        endingSeeYouAgainConfig.scale = 2.0f;
        
        endingSkipConfig.position.useRelative = false;
        endingSkipConfig.position.absoluteX = 880.0f;
        endingSkipConfig.position.absoluteY = 30.0f;
        endingSkipConfig.color = glm::vec3(1.0f, 1.0f, 1.0f);
        endingSkipConfig.scale = 1.2f;
        
        // Warp Tutorial UI設定
        warpTutorialTitleConfig.position.useRelative = true;
        warpTutorialTitleConfig.position.offsetX = -350.0f;
        warpTutorialTitleConfig.position.offsetY = -200.0f;
        warpTutorialTitleConfig.color = glm::vec3(1.0f, 1.0f, 0.0f);
        warpTutorialTitleConfig.scale = 2.0f;
        
        warpTutorialDescription1Config.position.useRelative = true;
        warpTutorialDescription1Config.position.offsetX = -300.0f;
        warpTutorialDescription1Config.position.offsetY = -70.0f;
        warpTutorialDescription1Config.color = glm::vec3(1.0f, 1.0f, 1.0f);
        warpTutorialDescription1Config.scale = 1.5f;
        
        warpTutorialDescription2Config.position.useRelative = true;
        warpTutorialDescription2Config.position.offsetX = -200.0f;
        warpTutorialDescription2Config.position.offsetY = 20.0f;
        warpTutorialDescription2Config.color = glm::vec3(1.0f, 1.0f, 1.0f);
        warpTutorialDescription2Config.scale = 1.5f;
        
        warpTutorialEnterButtonConfig.position.useRelative = true;
        warpTutorialEnterButtonConfig.position.offsetX = -200.0f;
        warpTutorialEnterButtonConfig.position.offsetY = 150.0f;
        warpTutorialEnterButtonConfig.color = glm::vec3(0.2f, 0.8f, 0.2f);
        warpTutorialEnterButtonConfig.scale = 1.5f;
        
        // Stage 0 Tutorial UI設定
        stage0TutorialLine1Config.position.useRelative = true;
        stage0TutorialLine1Config.position.offsetX = -250.0f;
        stage0TutorialLine1Config.position.offsetY = -250.0f;
        stage0TutorialLine1Config.color = glm::vec3(1.0f, 1.0f, 1.0f);
        stage0TutorialLine1Config.scale = 2.0f;
        
        stage0TutorialLine2Config.position.useRelative = true;
        stage0TutorialLine2Config.position.offsetX = -380.0f;
        stage0TutorialLine2Config.position.offsetY = -120.0f;
        stage0TutorialLine2Config.color = glm::vec3(1.0f, 1.0f, 1.0f);
        stage0TutorialLine2Config.scale = 1.5f;
        
        stage0TutorialLine3Config.position.useRelative = true;
        stage0TutorialLine3Config.position.offsetX = -320.0f;
        stage0TutorialLine3Config.position.offsetY = -20.0f;
        stage0TutorialLine3Config.color = glm::vec3(1.0f, 1.0f, 1.0f);
        stage0TutorialLine3Config.scale = 1.5f;
        
        stage0TutorialLine4Config.position.useRelative = true;
        stage0TutorialLine4Config.position.offsetX = -580.0f;
        stage0TutorialLine4Config.position.offsetY = 80.0f;
        stage0TutorialLine4Config.color = glm::vec3(1.0f, 1.0f, 1.0f);
        stage0TutorialLine4Config.scale = 1.5f;
        
        stage0TutorialLine5Config.position.useRelative = true;
        stage0TutorialLine5Config.position.offsetX = -400.0f;
        stage0TutorialLine5Config.position.offsetY = 180.0f;
        stage0TutorialLine5Config.color = glm::vec3(1.0f, 0.8f, 0.2f);
        stage0TutorialLine5Config.scale = 1.5f;
        
        stage0TutorialOkButtonConfig.position.useRelative = true;
        stage0TutorialOkButtonConfig.position.offsetX = -80.0f;
        stage0TutorialOkButtonConfig.position.offsetY = 280.0f;
        stage0TutorialOkButtonConfig.color = glm::vec3(0.2f, 0.8f, 0.2f);
        stage0TutorialOkButtonConfig.scale = 1.5f;
        
        // Easy Mode Explanation UI設定
        easyModeExplanationTitleConfig.position.useRelative = true;
        easyModeExplanationTitleConfig.position.offsetX = -350.0f;
        easyModeExplanationTitleConfig.position.offsetY = -300.0f;
        easyModeExplanationTitleConfig.color = glm::vec3(1.0f, 1.0f, 0.0f);
        easyModeExplanationTitleConfig.scale = 2.0f;
        
        easyModeExplanationDescription1Config.position.useRelative = true;
        easyModeExplanationDescription1Config.position.offsetX = -550.0f;
        easyModeExplanationDescription1Config.position.offsetY = -200.0f;
        easyModeExplanationDescription1Config.color = glm::vec3(1.0f, 1.0f, 1.0f);
        easyModeExplanationDescription1Config.scale = 1.5f;
        
        easyModeExplanationDescription2Config.position.useRelative = true;
        easyModeExplanationDescription2Config.position.offsetX = -150.0f;
        easyModeExplanationDescription2Config.position.offsetY = -80.0f;
        easyModeExplanationDescription2Config.color = glm::vec3(1.0f, 1.0f, 1.0f);
        easyModeExplanationDescription2Config.scale = 1.5f;
        
        easyModeExplanationItem1Config.position.useRelative = true;
        easyModeExplanationItem1Config.position.offsetX = -400.0f;
        easyModeExplanationItem1Config.position.offsetY = 0.0f;
        easyModeExplanationItem1Config.color = glm::vec3(1.0f, 1.0f, 1.0f);
        easyModeExplanationItem1Config.scale = 1.2f;
        
        easyModeExplanationItem2Config.position.useRelative = true;
        easyModeExplanationItem2Config.position.offsetX = -400.0f;
        easyModeExplanationItem2Config.position.offsetY = 60.0f;
        easyModeExplanationItem2Config.color = glm::vec3(1.0f, 1.0f, 1.0f);
        easyModeExplanationItem2Config.scale = 1.2f;
        
        easyModeExplanationItem3Config.position.useRelative = true;
        easyModeExplanationItem3Config.position.offsetX = -400.0f;
        easyModeExplanationItem3Config.position.offsetY = 120.0f;
        easyModeExplanationItem3Config.color = glm::vec3(1.0f, 1.0f, 1.0f);
        easyModeExplanationItem3Config.scale = 1.2f;
        
        easyModeExplanationItem4aConfig.position.useRelative = true;
        easyModeExplanationItem4aConfig.position.offsetX = -400.0f;
        easyModeExplanationItem4aConfig.position.offsetY = 180.0f;
        easyModeExplanationItem4aConfig.color = glm::vec3(1.0f, 1.0f, 1.0f);
        easyModeExplanationItem4aConfig.scale = 1.2f;
        
        easyModeExplanationItem4bConfig.position.useRelative = true;
        easyModeExplanationItem4bConfig.position.offsetX = -380.0f;
        easyModeExplanationItem4bConfig.position.offsetY = 240.0f;
        easyModeExplanationItem4bConfig.color = glm::vec3(1.0f, 1.0f, 1.0f);
        easyModeExplanationItem4bConfig.scale = 1.2f;
        
        easyModeExplanationOkButtonConfig.position.useRelative = true;
        easyModeExplanationOkButtonConfig.position.offsetX = -150.0f;
        easyModeExplanationOkButtonConfig.position.offsetY = 320.0f;
        easyModeExplanationOkButtonConfig.color = glm::vec3(0.2f, 0.8f, 0.2f);
        easyModeExplanationOkButtonConfig.scale = 1.5f;
        
        // Countdown UI設定
        countdownNumberConfig.position.useRelative = true;
        countdownNumberConfig.position.offsetX = -100.0f;
        countdownNumberConfig.position.offsetY = -100.0f;
        countdownNumberConfig.color = glm::vec3(1.0f, 1.0f, 1.0f);
        countdownNumberConfig.scale = 5.0f;
        
        // Stage Selection Assist UI設定
        stageSelectionAssistTextConfig.position.useRelative = true;
        stageSelectionAssistTextConfig.position.offsetX = -200.0f;
        stageSelectionAssistTextConfig.position.offsetY = 100.0f;
        stageSelectionAssistTextConfig.color = glm::vec3(1.0f, 1.0f, 1.0f);
        stageSelectionAssistTextConfig.scale = 1.3f;
        
        // Staff Roll UI設定
        staffRollSkipConfig.position.useRelative = true;
        staffRollSkipConfig.position.offsetX = -70.0f;
        staffRollSkipConfig.position.offsetY = -650.0f;
        staffRollSkipConfig.color = glm::vec3(1.0f, 1.0f, 1.0f);
        staffRollSkipConfig.scale = 1.5f;
        
        staffRollTitleConfig.position.useRelative = true;
        staffRollTitleConfig.position.offsetX = -600.0f;
        staffRollTitleConfig.position.offsetY = 0.0f;
        staffRollTitleConfig.color = glm::vec3(1.0f, 1.0f, 1.0f);
        staffRollTitleConfig.scale = 2.5f;
        
        staffRollRoleConfig.position.useRelative = true;
        staffRollRoleConfig.position.offsetX = -750.0f;
        staffRollRoleConfig.position.offsetY = 0.0f;
        staffRollRoleConfig.color = glm::vec3(1.0f, 1.0f, 1.0f);
        staffRollRoleConfig.scale = 2.0f;
        
        staffRollNameConfig.position.useRelative = true;
        staffRollNameConfig.position.offsetX = -500.0f;
        staffRollNameConfig.position.offsetY = 0.0f;
        staffRollNameConfig.color = glm::vec3(1.0f, 1.0f, 1.0f);
        staffRollNameConfig.scale = 2.0f;
        
        staffRollSpacing = 100.0f;
        
        // Game UI設定
        gameUITimeDisplayConfig.position.useRelative = false;
        gameUITimeDisplayConfig.position.absoluteX = 1170.0f;
        gameUITimeDisplayConfig.position.absoluteY = 30.0f;
        gameUITimeDisplayConfig.normalColor = glm::vec3(1.0f, 1.0f, 1.0f);
        gameUITimeDisplayConfig.warningColor = glm::vec3(1.0f, 0.5f, 0.0f);
        gameUITimeDisplayConfig.scale = 3.0f;
        
        gameUITimeAttackDisplayConfig.position.useRelative = false;
        gameUITimeAttackDisplayConfig.position.absoluteX = 1170.0f;
        gameUITimeAttackDisplayConfig.position.absoluteY = 30.0f;
        gameUITimeAttackDisplayConfig.color = glm::vec3(1.0f, 1.0f, 1.0f);
        gameUITimeAttackDisplayConfig.scale = 2.1f;
        
        gameUIBestTimeConfig.position.useRelative = false;
        gameUIBestTimeConfig.position.absoluteX = 1170.0f;
        gameUIBestTimeConfig.position.absoluteY = 60.0f;
        gameUIBestTimeConfig.color = glm::vec3(1.0f, 1.0f, 1.0f);
        gameUIBestTimeConfig.scale = 1.8f;
        
        gameUIGoalDisplayConfig.position.useRelative = false;
        gameUIGoalDisplayConfig.position.absoluteX = 962.0f;
        gameUIGoalDisplayConfig.position.absoluteY = 65.0f;
        gameUIGoalDisplayConfig.color = glm::vec3(1.0f, 1.0f, 1.0f);
        gameUIGoalDisplayConfig.scale = 1.0f;
        
        gameUIGoalTime5sConfig.position.useRelative = false;
        gameUIGoalTime5sConfig.position.absoluteX = 1040.0f;
        gameUIGoalTime5sConfig.position.absoluteY = 65.0f;
        gameUIGoalTime5sConfig.color = glm::vec3(1.0f, 1.0f, 1.0f);
        gameUIGoalTime5sConfig.scale = 1.0f;
        
        gameUIGoalTime10sConfig.position.useRelative = false;
        gameUIGoalTime10sConfig.position.absoluteX = 1110.0f;
        gameUIGoalTime10sConfig.position.absoluteY = 65.0f;
        gameUIGoalTime10sConfig.color = glm::vec3(1.0f, 1.0f, 1.0f);
        gameUIGoalTime10sConfig.scale = 1.0f;
        
        gameUIGoalTime20sConfig.position.useRelative = false;
        gameUIGoalTime20sConfig.position.absoluteX = 1110.0f;
        gameUIGoalTime20sConfig.position.absoluteY = 65.0f;
        gameUIGoalTime20sConfig.color = glm::vec3(1.0f, 1.0f, 1.0f);
        gameUIGoalTime20sConfig.scale = 1.0f;
        
        gameUIStarsConfig.position.useRelative = false;
        gameUIStarsConfig.position.absoluteX = 980.0f;
        gameUIStarsConfig.position.absoluteY = 40.0f;
        gameUIStarsConfig.spacing = 70.0f;
        gameUIStarsConfig.selectedColor = glm::vec3(1.0f, 1.0f, 0.0f);
        gameUIStarsConfig.unselectedColor = glm::vec3(0.5f, 0.5f, 0.5f);
        gameUIStarsConfig.scale = 1.5f;
        
        gameUIHeartsConfig.position.useRelative = false;
        gameUIHeartsConfig.position.absoluteX = 200.0f;
        gameUIHeartsConfig.position.absoluteY = 45.0f;
        gameUIHeartsConfig.spacing = 40.0f;
        gameUIHeartsConfig.selectedColor = glm::vec3(1.0f, 0.3f, 0.3f);
        gameUIHeartsConfig.unselectedColor = glm::vec3(0.3f, 0.3f, 0.3f);
        gameUIHeartsConfig.scale = 1.0f;
        
        // Skills設定
        gameUITimeStopSkillConfig.position.useRelative = false;
        gameUITimeStopSkillConfig.position.absoluteX = 30.0f;
        gameUITimeStopSkillConfig.position.absoluteY = 650.0f;
        gameUITimeStopSkillConfig.countOffset = 50.0f;
        gameUITimeStopSkillConfig.instructionOffset = 20.0f;
        gameUITimeStopSkillConfig.color = glm::vec3(1.0f, 1.0f, 1.0f);
        gameUITimeStopSkillConfig.disabledColor = glm::vec3(0.5f, 0.5f, 0.5f);
        gameUITimeStopSkillConfig.activeColor = glm::vec3(0.5f, 0.5f, 1.0f);
        gameUITimeStopSkillConfig.scale = 1.0f;
        gameUITimeStopSkillConfig.countScale = 3.0f;
        gameUITimeStopSkillConfig.instructionScale = 1.2f;
        
        gameUIDoubleJumpSkillConfig.position.useRelative = false;
        gameUIDoubleJumpSkillConfig.position.absoluteX = 230.0f;
        gameUIDoubleJumpSkillConfig.position.absoluteY = 650.0f;
        gameUIDoubleJumpSkillConfig.countOffset = 70.0f;
        gameUIDoubleJumpSkillConfig.instructionOffset = -25.0f;
        gameUIDoubleJumpSkillConfig.color = glm::vec3(1.0f, 1.0f, 1.0f);
        gameUIDoubleJumpSkillConfig.disabledColor = glm::vec3(0.5f, 0.5f, 0.5f);
        gameUIDoubleJumpSkillConfig.activeColor = glm::vec3(0.5f, 0.5f, 1.0f);
        gameUIDoubleJumpSkillConfig.scale = 1.0f;
        gameUIDoubleJumpSkillConfig.countScale = 3.0f;
        gameUIDoubleJumpSkillConfig.instructionScale = 1.2f;
        
        gameUIHeartFeelSkillConfig.position.useRelative = false;
        gameUIHeartFeelSkillConfig.position.absoluteX = 460.0f;
        gameUIHeartFeelSkillConfig.position.absoluteY = 650.0f;
        gameUIHeartFeelSkillConfig.countOffset = 60.0f;
        gameUIHeartFeelSkillConfig.instructionOffset = 30.0f;
        gameUIHeartFeelSkillConfig.color = glm::vec3(1.0f, 1.0f, 1.0f);
        gameUIHeartFeelSkillConfig.disabledColor = glm::vec3(0.5f, 0.5f, 0.5f);
        gameUIHeartFeelSkillConfig.activeColor = glm::vec3(0.5f, 0.5f, 1.0f);
        gameUIHeartFeelSkillConfig.scale = 1.0f;
        gameUIHeartFeelSkillConfig.countScale = 3.0f;
        gameUIHeartFeelSkillConfig.instructionScale = 1.2f;
        
        gameUIFreeCameraSkillConfig.position.useRelative = false;
        gameUIFreeCameraSkillConfig.position.absoluteX = 850.0f;
        gameUIFreeCameraSkillConfig.position.absoluteY = 650.0f;
        gameUIFreeCameraSkillConfig.countOffset = 60.0f;
        gameUIFreeCameraSkillConfig.instructionOffset = 40.0f;
        gameUIFreeCameraSkillConfig.color = glm::vec3(1.0f, 1.0f, 1.0f);
        gameUIFreeCameraSkillConfig.disabledColor = glm::vec3(0.5f, 0.5f, 0.5f);
        gameUIFreeCameraSkillConfig.activeColor = glm::vec3(0.5f, 0.5f, 1.0f);
        gameUIFreeCameraSkillConfig.scale = 1.0f;
        gameUIFreeCameraSkillConfig.countScale = 3.0f;
        gameUIFreeCameraSkillConfig.instructionScale = 1.2f;
        gameUIFreeCameraSkillConfig.activePosition.useRelative = false;
        gameUIFreeCameraSkillConfig.activePosition.absoluteX = 550.0f;
        gameUIFreeCameraSkillConfig.activePosition.absoluteY = 490.0f;
        gameUIFreeCameraSkillConfig.activeScale = 2.0f;
        
        gameUIBurstJumpSkillConfig.position.useRelative = false;
        gameUIBurstJumpSkillConfig.position.absoluteX = 650.0f;
        gameUIBurstJumpSkillConfig.position.absoluteY = 650.0f;
        gameUIBurstJumpSkillConfig.countOffset = 50.0f;
        gameUIBurstJumpSkillConfig.instructionOffset = 25.0f;
        gameUIBurstJumpSkillConfig.color = glm::vec3(1.0f, 1.0f, 1.0f);
        gameUIBurstJumpSkillConfig.disabledColor = glm::vec3(0.5f, 0.5f, 0.5f);
        gameUIBurstJumpSkillConfig.activeColor = glm::vec3(0.5f, 0.5f, 1.0f);
        gameUIBurstJumpSkillConfig.scale = 1.0f;
        gameUIBurstJumpSkillConfig.countScale = 3.0f;
        gameUIBurstJumpSkillConfig.instructionScale = 1.2f;
    }
    
    bool UIConfigManager::loadConfig(const std::string& filepath) {
        configFilePath = filepath;
        setDefaultValues();  // デフォルト値を設定
        
        std::ifstream file(filepath);
        if (!file.is_open()) {
            // 代替パスを試す
            std::string altPath = "../" + filepath;
            file.open(altPath);
            if (!file.is_open()) {
                printf("UI Config: File not found, using default values: %s\n", filepath.c_str());
                configLoaded = false;
                return false;
            }
            configFilePath = altPath;
        }
        
        try {
            nlohmann::json jsonData;
            file >> jsonData;
            file.close();
            
            // JSONデータをキャッシュ（動的アクセス用）
            cachedJsonData = jsonData;
            
            // ステージ情報
            if (jsonData.contains("stageInfo")) {
                auto& cfg = jsonData["stageInfo"];
                if (cfg.contains("position")) {
                    auto& pos = cfg["position"];
                    if (pos.contains("absoluteX")) stageInfoConfig.position.absoluteX = pos["absoluteX"];
                    if (pos.contains("absoluteY")) stageInfoConfig.position.absoluteY = pos["absoluteY"];
                    if (pos.contains("offsetX")) stageInfoConfig.position.offsetX = pos["offsetX"];
                    if (pos.contains("offsetY")) stageInfoConfig.position.offsetY = pos["offsetY"];
                    if (pos.contains("useRelative")) stageInfoConfig.position.useRelative = pos["useRelative"];
                }
                if (cfg.contains("color")) {
                    stageInfoConfig.color = glm::vec3(cfg["color"][0], cfg["color"][1], cfg["color"][2]);
                }
                if (cfg.contains("scale")) stageInfoConfig.scale = cfg["scale"];
            }
            
            // 速度倍率表示
            if (jsonData.contains("speedDisplay")) {
                auto& cfg = jsonData["speedDisplay"];
                if (cfg.contains("position")) {
                    auto& pos = cfg["position"];
                    if (pos.contains("absoluteX")) speedDisplayConfig.position.absoluteX = pos["absoluteX"];
                    if (pos.contains("absoluteY")) speedDisplayConfig.position.absoluteY = pos["absoluteY"];
                    if (pos.contains("offsetX")) speedDisplayConfig.position.offsetX = pos["offsetX"];
                    if (pos.contains("offsetY")) speedDisplayConfig.position.offsetY = pos["offsetY"];
                    if (pos.contains("useRelative")) speedDisplayConfig.position.useRelative = pos["useRelative"];
                }
                if (cfg.contains("color")) {
                    speedDisplayConfig.color = glm::vec3(cfg["color"][0], cfg["color"][1], cfg["color"][2]);
                }
                if (cfg.contains("scale")) speedDisplayConfig.scale = cfg["scale"];
            }
            
            // PRESS T表示（通常モード）
            if (jsonData.contains("pressT")) {
                auto& cfg = jsonData["pressT"];
                if (cfg.contains("position")) {
                    auto& pos = cfg["position"];
                    if (pos.contains("absoluteX")) pressTConfig.position.absoluteX = pos["absoluteX"];
                    if (pos.contains("absoluteY")) pressTConfig.position.absoluteY = pos["absoluteY"];
                    if (pos.contains("offsetX")) pressTConfig.position.offsetX = pos["offsetX"];
                    if (pos.contains("offsetY")) pressTConfig.position.offsetY = pos["offsetY"];
                    if (pos.contains("useRelative")) pressTConfig.position.useRelative = pos["useRelative"];
                }
                if (cfg.contains("color")) {
                    pressTConfig.color = glm::vec3(cfg["color"][0], cfg["color"][1], cfg["color"][2]);
                }
                if (cfg.contains("scale")) pressTConfig.scale = cfg["scale"];
            }
            
            // リプレイUI設定
            if (jsonData.contains("replayUI")) {
                auto& replay = jsonData["replayUI"];
                
                if (replay.contains("pauseMark")) {
                    auto& cfg = replay["pauseMark"];
                    if (cfg.contains("position")) {
                        auto& pos = cfg["position"];
                        if (pos.contains("offsetX")) replayPauseMarkConfig.position.offsetX = pos["offsetX"];
                        if (pos.contains("offsetY")) replayPauseMarkConfig.position.offsetY = pos["offsetY"];
                    }
                    if (cfg.contains("color")) {
                        replayPauseMarkConfig.color = glm::vec3(cfg["color"][0], cfg["color"][1], cfg["color"][2]);
                    }
                    if (cfg.contains("scale")) replayPauseMarkConfig.scale = cfg["scale"];
                }
                
                if (replay.contains("rewindMark")) {
                    auto& cfg = replay["rewindMark"];
                    if (cfg.contains("position")) {
                        auto& pos = cfg["position"];
                        if (pos.contains("offsetX")) replayRewindMarkConfig.position.offsetX = pos["offsetX"];
                        if (pos.contains("offsetY")) replayRewindMarkConfig.position.offsetY = pos["offsetY"];
                    }
                    if (cfg.contains("color")) {
                        replayRewindMarkConfig.color = glm::vec3(cfg["color"][0], cfg["color"][1], cfg["color"][2]);
                    }
                    if (cfg.contains("scale")) replayRewindMarkConfig.scale = cfg["scale"];
                }
                
                if (replay.contains("fastForwardMark")) {
                    auto& cfg = replay["fastForwardMark"];
                    if (cfg.contains("position")) {
                        auto& pos = cfg["position"];
                        if (pos.contains("offsetX")) replayFastForwardMarkConfig.position.offsetX = pos["offsetX"];
                        if (pos.contains("offsetY")) replayFastForwardMarkConfig.position.offsetY = pos["offsetY"];
                    }
                    if (cfg.contains("color")) {
                        replayFastForwardMarkConfig.color = glm::vec3(cfg["color"][0], cfg["color"][1], cfg["color"][2]);
                    }
                    if (cfg.contains("scale")) replayFastForwardMarkConfig.scale = cfg["scale"];
                }
                
                if (replay.contains("speedLabel")) {
                    auto& cfg = replay["speedLabel"];
                    if (cfg.contains("position")) {
                        auto& pos = cfg["position"];
                        if (pos.contains("offsetX")) replaySpeedLabelConfig.position.offsetX = pos["offsetX"];
                        if (pos.contains("offsetY")) replaySpeedLabelConfig.position.offsetY = pos["offsetY"];
                    }
                    if (cfg.contains("color")) {
                        replaySpeedLabelConfig.color = glm::vec3(cfg["color"][0], cfg["color"][1], cfg["color"][2]);
                    }
                    if (cfg.contains("scale")) replaySpeedLabelConfig.scale = cfg["scale"];
                }
                
                if (replay.contains("speedDisplay")) {
                    auto& cfg = replay["speedDisplay"];
                    if (cfg.contains("position")) {
                        auto& pos = cfg["position"];
                        if (pos.contains("offsetX")) replaySpeedDisplayConfig.position.offsetX = pos["offsetX"];
                        if (pos.contains("offsetY")) replaySpeedDisplayConfig.position.offsetY = pos["offsetY"];
                    }
                    if (cfg.contains("color")) {
                        replaySpeedDisplayConfig.color = glm::vec3(cfg["color"][0], cfg["color"][1], cfg["color"][2]);
                    }
                    if (cfg.contains("scale")) replaySpeedDisplayConfig.scale = cfg["scale"];
                }
                
                if (replay.contains("pressT")) {
                    auto& cfg = replay["pressT"];
                    if (cfg.contains("position")) {
                        auto& pos = cfg["position"];
                        if (pos.contains("offsetX")) replayPressTConfig.position.offsetX = pos["offsetX"];
                        if (pos.contains("offsetY")) replayPressTConfig.position.offsetY = pos["offsetY"];
                    }
                    if (cfg.contains("color")) {
                        replayPressTConfig.color = glm::vec3(cfg["color"][0], cfg["color"][1], cfg["color"][2]);
                    }
                    if (cfg.contains("scale")) replayPressTConfig.scale = cfg["scale"];
                }
                
                if (replay.contains("instructions")) {
                    auto& cfg = replay["instructions"];
                    if (cfg.contains("position")) {
                        auto& pos = cfg["position"];
                        if (pos.contains("offsetX")) replayInstructionsConfig.position.offsetX = pos["offsetX"];
                        if (pos.contains("offsetY")) replayInstructionsConfig.position.offsetY = pos["offsetY"];
                    }
                    if (cfg.contains("color")) {
                        replayInstructionsConfig.color = glm::vec3(cfg["color"][0], cfg["color"][1], cfg["color"][2]);
                    }
                    if (cfg.contains("scale")) replayInstructionsConfig.scale = cfg["scale"];
                }
            }
            
            // ステージ選択フィールドUI
            if (jsonData.contains("stageSelection")) {
                auto& sel = jsonData["stageSelection"];
                
                if (sel.contains("worldTitle")) {
                    auto& cfg = sel["worldTitle"];
                    if (cfg.contains("position")) {
                        auto& pos = cfg["position"];
                        if (pos.contains("offsetX")) worldTitleConfig.position.offsetX = pos["offsetX"];
                        if (pos.contains("offsetY")) worldTitleConfig.position.offsetY = pos["offsetY"];
                    }
                    if (cfg.contains("color")) {
                        worldTitleConfig.color = glm::vec3(cfg["color"][0], cfg["color"][1], cfg["color"][2]);
                    }
                    if (cfg.contains("scale")) worldTitleConfig.scale = cfg["scale"];
                }
                
                if (sel.contains("starCount")) {
                    auto& cfg = sel["starCount"];
                    if (cfg.contains("position")) {
                        auto& pos = cfg["position"];
                        if (pos.contains("absoluteX")) starCountConfig.position.absoluteX = pos["absoluteX"];
                        if (pos.contains("absoluteY")) starCountConfig.position.absoluteY = pos["absoluteY"];
                    }
                    if (cfg.contains("color")) {
                        starCountConfig.color = glm::vec3(cfg["color"][0], cfg["color"][1], cfg["color"][2]);
                    }
                    if (cfg.contains("scale")) starCountConfig.scale = cfg["scale"];
                }
                
                if (sel.contains("modeText")) {
                    auto& cfg = sel["modeText"];
                    if (cfg.contains("position")) {
                        auto& pos = cfg["position"];
                        if (pos.contains("absoluteX")) modeTextConfig.position.absoluteX = pos["absoluteX"];
                        if (pos.contains("absoluteY")) modeTextConfig.position.absoluteY = pos["absoluteY"];
                    }
                    if (cfg.contains("color")) {
                        modeTextConfig.color = glm::vec3(cfg["color"][0], cfg["color"][1], cfg["color"][2]);
                    }
                    if (cfg.contains("scale")) modeTextConfig.scale = cfg["scale"];
                }
                
                if (sel.contains("pressE")) {
                    auto& cfg = sel["pressE"];
                    if (cfg.contains("position")) {
                        auto& pos = cfg["position"];
                        if (pos.contains("absoluteX")) pressEConfig.position.absoluteX = pos["absoluteX"];
                        if (pos.contains("absoluteY")) pressEConfig.position.absoluteY = pos["absoluteY"];
                    }
                    if (cfg.contains("color")) {
                        pressEConfig.color = glm::vec3(cfg["color"][0], cfg["color"][1], cfg["color"][2]);
                    }
                    if (cfg.contains("scale")) pressEConfig.scale = cfg["scale"];
                }
                
                if (sel.contains("starIcon")) {
                    auto& cfg = sel["starIcon"];
                    if (cfg.contains("position")) {
                        auto& pos = cfg["position"];
                        if (pos.contains("absoluteX")) starIconConfig.position.absoluteX = pos["absoluteX"];
                        if (pos.contains("absoluteY")) starIconConfig.position.absoluteY = pos["absoluteY"];
                        if (pos.contains("useRelative")) starIconConfig.position.useRelative = pos["useRelative"];
                    }
                    if (cfg.contains("color")) {
                        starIconConfig.color = glm::vec3(cfg["color"][0], cfg["color"][1], cfg["color"][2]);
                    }
                    if (cfg.contains("scale")) starIconConfig.scale = cfg["scale"];
                }
                
                if (sel.contains("controlsText")) {
                    auto& cfg = sel["controlsText"];
                    if (cfg.contains("position")) {
                        auto& pos = cfg["position"];
                        if (pos.contains("absoluteX")) controlsTextConfig.position.absoluteX = pos["absoluteX"];
                        if (pos.contains("absoluteY")) controlsTextConfig.position.absoluteY = pos["absoluteY"];
                        if (pos.contains("offsetY")) controlsTextConfig.position.offsetY = pos["offsetY"];
                        if (pos.contains("useRelative")) controlsTextConfig.position.useRelative = pos["useRelative"];
                    }
                    if (cfg.contains("color")) {
                        controlsTextConfig.color = glm::vec3(cfg["color"][0], cfg["color"][1], cfg["color"][2]);
                    }
                    if (cfg.contains("scale")) controlsTextConfig.scale = cfg["scale"];
                }
            }
            
            // Ready画面UI
            if (jsonData.contains("readyScreen")) {
                auto& ready = jsonData["readyScreen"];
                
                if (ready.contains("readyText")) {
                    auto& cfg = ready["readyText"];
                    if (cfg.contains("position")) {
                        auto& pos = cfg["position"];
                        if (pos.contains("offsetX")) readyTextConfig.position.offsetX = pos["offsetX"];
                        if (pos.contains("offsetY")) readyTextConfig.position.offsetY = pos["offsetY"];
                    }
                    if (cfg.contains("color")) {
                        readyTextConfig.color = glm::vec3(cfg["color"][0], cfg["color"][1], cfg["color"][2]);
                    }
                    if (cfg.contains("scale")) readyTextConfig.scale = cfg["scale"];
                }
                
                if (ready.contains("playSpeedLabel")) {
                    auto& cfg = ready["playSpeedLabel"];
                    if (cfg.contains("position")) {
                        auto& pos = cfg["position"];
                        if (pos.contains("offsetX")) playSpeedLabelConfig.position.offsetX = pos["offsetX"];
                        if (pos.contains("offsetY")) playSpeedLabelConfig.position.offsetY = pos["offsetY"];
                    }
                    if (cfg.contains("color")) {
                        playSpeedLabelConfig.color = glm::vec3(cfg["color"][0], cfg["color"][1], cfg["color"][2]);
                    }
                    if (cfg.contains("scale")) playSpeedLabelConfig.scale = cfg["scale"];
                }
                
                if (ready.contains("speedOptions")) {
                    auto& cfg = ready["speedOptions"];
                    if (cfg.contains("position")) {
                        auto& pos = cfg["position"];
                        if (pos.contains("offsetX")) speedOptionsConfig.position.offsetX = pos["offsetX"];
                        if (pos.contains("offsetY")) speedOptionsConfig.position.offsetY = pos["offsetY"];
                        if (pos.contains("spacing")) speedOptionsConfig.spacing = pos["spacing"];
                    }
                    if (cfg.contains("selectedColor")) {
                        speedOptionsConfig.selectedColor = glm::vec3(cfg["selectedColor"][0], cfg["selectedColor"][1], cfg["selectedColor"][2]);
                    }
                    if (cfg.contains("unselectedColor")) {
                        speedOptionsConfig.unselectedColor = glm::vec3(cfg["unselectedColor"][0], cfg["unselectedColor"][1], cfg["unselectedColor"][2]);
                    }
                    if (cfg.contains("scale")) speedOptionsConfig.scale = cfg["scale"];
                }
                
                if (ready.contains("pressT")) {
                    auto& cfg = ready["pressT"];
                    if (cfg.contains("position")) {
                        auto& pos = cfg["position"];
                        if (pos.contains("offsetX")) readyPressTConfig.position.offsetX = pos["offsetX"];
                        if (pos.contains("offsetY")) readyPressTConfig.position.offsetY = pos["offsetY"];
                    }
                    if (cfg.contains("color")) {
                        readyPressTConfig.color = glm::vec3(cfg["color"][0], cfg["color"][1], cfg["color"][2]);
                    }
                    if (cfg.contains("scale")) readyPressTConfig.scale = cfg["scale"];
                }
                
                if (ready.contains("tpsFpsLabel")) {
                    auto& cfg = ready["tpsFpsLabel"];
                    if (cfg.contains("position")) {
                        auto& pos = cfg["position"];
                        if (pos.contains("offsetX")) tpsFpsLabelConfig.position.offsetX = pos["offsetX"];
                        if (pos.contains("offsetY")) tpsFpsLabelConfig.position.offsetY = pos["offsetY"];
                    }
                    if (cfg.contains("color")) {
                        tpsFpsLabelConfig.color = glm::vec3(cfg["color"][0], cfg["color"][1], cfg["color"][2]);
                    }
                    if (cfg.contains("scale")) tpsFpsLabelConfig.scale = cfg["scale"];
                }
                
                if (ready.contains("tpsOption")) {
                    auto& cfg = ready["tpsOption"];
                    if (cfg.contains("position")) {
                        auto& pos = cfg["position"];
                        if (pos.contains("offsetX")) tpsOptionConfig.position.offsetX = pos["offsetX"];
                        if (pos.contains("offsetY")) tpsOptionConfig.position.offsetY = pos["offsetY"];
                    }
                    if (cfg.contains("selectedColor")) {
                        tpsOptionConfig.selectedColor = glm::vec3(cfg["selectedColor"][0], cfg["selectedColor"][1], cfg["selectedColor"][2]);
                    }
                    if (cfg.contains("unselectedColor")) {
                        tpsOptionConfig.unselectedColor = glm::vec3(cfg["unselectedColor"][0], cfg["unselectedColor"][1], cfg["unselectedColor"][2]);
                    }
                    if (cfg.contains("scale")) tpsOptionConfig.scale = cfg["scale"];
                }
                
                if (ready.contains("fpsOption")) {
                    auto& cfg = ready["fpsOption"];
                    if (cfg.contains("position")) {
                        auto& pos = cfg["position"];
                        if (pos.contains("offsetX")) fpsOptionConfig.position.offsetX = pos["offsetX"];
                        if (pos.contains("offsetY")) fpsOptionConfig.position.offsetY = pos["offsetY"];
                    }
                    if (cfg.contains("selectedColor")) {
                        fpsOptionConfig.selectedColor = glm::vec3(cfg["selectedColor"][0], cfg["selectedColor"][1], cfg["selectedColor"][2]);
                    }
                    if (cfg.contains("unselectedColor")) {
                        fpsOptionConfig.unselectedColor = glm::vec3(cfg["unselectedColor"][0], cfg["unselectedColor"][1], cfg["unselectedColor"][2]);
                    }
                    if (cfg.contains("scale")) fpsOptionConfig.scale = cfg["scale"];
                }
                
                if (ready.contains("pressF")) {
                    auto& cfg = ready["pressF"];
                    if (cfg.contains("position")) {
                        auto& pos = cfg["position"];
                        if (pos.contains("offsetX")) readyPressFConfig.position.offsetX = pos["offsetX"];
                        if (pos.contains("offsetY")) readyPressFConfig.position.offsetY = pos["offsetY"];
                    }
                    if (cfg.contains("color")) {
                        readyPressFConfig.color = glm::vec3(cfg["color"][0], cfg["color"][1], cfg["color"][2]);
                    }
                    if (cfg.contains("scale")) readyPressFConfig.scale = cfg["scale"];
                }
                
                if (ready.contains("confirm")) {
                    auto& cfg = ready["confirm"];
                    if (cfg.contains("position")) {
                        auto& pos = cfg["position"];
                        if (pos.contains("offsetX")) confirmConfig.position.offsetX = pos["offsetX"];
                        if (pos.contains("offsetY")) confirmConfig.position.offsetY = pos["offsetY"];
                    }
                    if (cfg.contains("color")) {
                        confirmConfig.color = glm::vec3(cfg["color"][0], cfg["color"][1], cfg["color"][2]);
                    }
                    if (cfg.contains("scale")) confirmConfig.scale = cfg["scale"];
                }
            }
            
            // Stage Clear UI
            if (jsonData.contains("stageClear")) {
                auto& stageClear = jsonData["stageClear"];
                
                if (stageClear.contains("completedText")) {
                    auto& cfg = stageClear["completedText"];
                    if (cfg.contains("position")) {
                        auto& pos = cfg["position"];
                        if (pos.contains("offsetX")) stageClearCompletedTextConfig.position.offsetX = pos["offsetX"];
                        if (pos.contains("offsetY")) stageClearCompletedTextConfig.position.offsetY = pos["offsetY"];
                    }
                    if (cfg.contains("color")) {
                        stageClearCompletedTextConfig.color = glm::vec3(cfg["color"][0], cfg["color"][1], cfg["color"][2]);
                    }
                    if (cfg.contains("scale")) stageClearCompletedTextConfig.scale = cfg["scale"];
                }
                
                if (stageClear.contains("clearText")) {
                    auto& cfg = stageClear["clearText"];
                    if (cfg.contains("position")) {
                        auto& pos = cfg["position"];
                        if (pos.contains("offsetX")) stageClearClearTextConfig.position.offsetX = pos["offsetX"];
                        if (pos.contains("offsetY")) stageClearClearTextConfig.position.offsetY = pos["offsetY"];
                    }
                    if (cfg.contains("color")) {
                        stageClearClearTextConfig.color = glm::vec3(cfg["color"][0], cfg["color"][1], cfg["color"][2]);
                    }
                    if (cfg.contains("scale")) stageClearClearTextConfig.scale = cfg["scale"];
                }
                
                if (stageClear.contains("clearTime")) {
                    auto& cfg = stageClear["clearTime"];
                    if (cfg.contains("position")) {
                        auto& pos = cfg["position"];
                        if (pos.contains("offsetX")) stageClearClearTimeConfig.position.offsetX = pos["offsetX"];
                        if (pos.contains("offsetY")) stageClearClearTimeConfig.position.offsetY = pos["offsetY"];
                    }
                    if (cfg.contains("color")) {
                        stageClearClearTimeConfig.color = glm::vec3(cfg["color"][0], cfg["color"][1], cfg["color"][2]);
                    }
                    if (cfg.contains("scale")) stageClearClearTimeConfig.scale = cfg["scale"];
                }
                
                if (stageClear.contains("returnField")) {
                    auto& cfg = stageClear["returnField"];
                    if (cfg.contains("position")) {
                        auto& pos = cfg["position"];
                        if (pos.contains("offsetX")) stageClearReturnFieldConfig.position.offsetX = pos["offsetX"];
                        if (pos.contains("offsetY")) stageClearReturnFieldConfig.position.offsetY = pos["offsetY"];
                    }
                    if (cfg.contains("color")) {
                        stageClearReturnFieldConfig.color = glm::vec3(cfg["color"][0], cfg["color"][1], cfg["color"][2]);
                    }
                    if (cfg.contains("scale")) stageClearReturnFieldConfig.scale = cfg["scale"];
                }
                
                if (stageClear.contains("retry")) {
                    auto& cfg = stageClear["retry"];
                    if (cfg.contains("position")) {
                        auto& pos = cfg["position"];
                        if (pos.contains("offsetX")) stageClearRetryConfig.position.offsetX = pos["offsetX"];
                        if (pos.contains("offsetY")) stageClearRetryConfig.position.offsetY = pos["offsetY"];
                    }
                    if (cfg.contains("color")) {
                        stageClearRetryConfig.color = glm::vec3(cfg["color"][0], cfg["color"][1], cfg["color"][2]);
                    }
                    if (cfg.contains("scale")) stageClearRetryConfig.scale = cfg["scale"];
                }
                
                if (stageClear.contains("stars")) {
                    auto& cfg = stageClear["stars"];
                    if (cfg.contains("position")) {
                        auto& pos = cfg["position"];
                        if (pos.contains("offsetX")) stageClearStarsConfig.position.offsetX = pos["offsetX"];
                        if (pos.contains("offsetY")) stageClearStarsConfig.position.offsetY = pos["offsetY"];
                        if (pos.contains("spacing")) stageClearStarsConfig.spacing = pos["spacing"];
                    }
                    if (cfg.contains("activeColor")) {
                        stageClearStarsConfig.selectedColor = glm::vec3(cfg["activeColor"][0], cfg["activeColor"][1], cfg["activeColor"][2]);
                    }
                    if (cfg.contains("inactiveColor")) {
                        stageClearStarsConfig.unselectedColor = glm::vec3(cfg["inactiveColor"][0], cfg["inactiveColor"][1], cfg["inactiveColor"][2]);
                    }
                    if (cfg.contains("scale")) stageClearStarsConfig.scale = cfg["scale"];
                }
            }
            
            // Game Over UI
            if (jsonData.contains("gameOver")) {
                auto& gameOver = jsonData["gameOver"];
                
                if (gameOver.contains("gameOverText")) {
                    auto& cfg = gameOver["gameOverText"];
                    if (cfg.contains("position")) {
                        auto& pos = cfg["position"];
                        if (pos.contains("offsetX")) gameOverTextConfig.position.offsetX = pos["offsetX"];
                        if (pos.contains("offsetY")) gameOverTextConfig.position.offsetY = pos["offsetY"];
                    }
                    if (cfg.contains("color")) {
                        gameOverTextConfig.color = glm::vec3(cfg["color"][0], cfg["color"][1], cfg["color"][2]);
                    }
                    if (cfg.contains("scale")) gameOverTextConfig.scale = cfg["scale"];
                }
                
                if (gameOver.contains("returnField")) {
                    auto& cfg = gameOver["returnField"];
                    if (cfg.contains("position")) {
                        auto& pos = cfg["position"];
                        if (pos.contains("offsetX")) gameOverReturnFieldConfig.position.offsetX = pos["offsetX"];
                        if (pos.contains("offsetY")) gameOverReturnFieldConfig.position.offsetY = pos["offsetY"];
                    }
                    if (cfg.contains("color")) {
                        gameOverReturnFieldConfig.color = glm::vec3(cfg["color"][0], cfg["color"][1], cfg["color"][2]);
                    }
                    if (cfg.contains("scale")) gameOverReturnFieldConfig.scale = cfg["scale"];
                }
                
                if (gameOver.contains("retry")) {
                    auto& cfg = gameOver["retry"];
                    if (cfg.contains("position")) {
                        auto& pos = cfg["position"];
                        if (pos.contains("offsetX")) gameOverRetryConfig.position.offsetX = pos["offsetX"];
                        if (pos.contains("offsetY")) gameOverRetryConfig.position.offsetY = pos["offsetY"];
                    }
                    if (cfg.contains("color")) {
                        gameOverRetryConfig.color = glm::vec3(cfg["color"][0], cfg["color"][1], cfg["color"][2]);
                    }
                    if (cfg.contains("scale")) gameOverRetryConfig.scale = cfg["scale"];
                }
            }
            
            // Mode Selection UI
            if (jsonData.contains("modeSelection")) {
                auto& modeSel = jsonData["modeSelection"];
                
                if (modeSel.contains("title")) {
                    auto& cfg = modeSel["title"];
                    if (cfg.contains("position")) {
                        auto& pos = cfg["position"];
                        if (pos.contains("offsetX")) modeSelectionTitleConfig.position.offsetX = pos["offsetX"];
                        if (pos.contains("offsetY")) modeSelectionTitleConfig.position.offsetY = pos["offsetY"];
                    }
                    if (cfg.contains("color")) {
                        modeSelectionTitleConfig.color = glm::vec3(cfg["color"][0], cfg["color"][1], cfg["color"][2]);
                    }
                    if (cfg.contains("scale")) modeSelectionTitleConfig.scale = cfg["scale"];
                }
                
                if (modeSel.contains("normalText")) {
                    auto& cfg = modeSel["normalText"];
                    if (cfg.contains("position")) {
                        auto& pos = cfg["position"];
                        if (pos.contains("offsetX")) modeSelectionNormalTextConfig.position.offsetX = pos["offsetX"];
                        if (pos.contains("offsetY")) modeSelectionNormalTextConfig.position.offsetY = pos["offsetY"];
                    }
                    if (cfg.contains("selectedColor")) {
                        modeSelectionNormalTextConfig.selectedColor = glm::vec3(cfg["selectedColor"][0], cfg["selectedColor"][1], cfg["selectedColor"][2]);
                    }
                    if (cfg.contains("unselectedColor")) {
                        modeSelectionNormalTextConfig.unselectedColor = glm::vec3(cfg["unselectedColor"][0], cfg["unselectedColor"][1], cfg["unselectedColor"][2]);
                    }
                    if (cfg.contains("scale")) modeSelectionNormalTextConfig.scale = cfg["scale"];
                }
                
                if (modeSel.contains("timeAttackText")) {
                    auto& cfg = modeSel["timeAttackText"];
                    if (cfg.contains("position")) {
                        auto& pos = cfg["position"];
                        if (pos.contains("offsetX")) modeSelectionTimeAttackTextConfig.position.offsetX = pos["offsetX"];
                        if (pos.contains("offsetY")) modeSelectionTimeAttackTextConfig.position.offsetY = pos["offsetY"];
                    }
                    if (cfg.contains("selectedColor")) {
                        modeSelectionTimeAttackTextConfig.selectedColor = glm::vec3(cfg["selectedColor"][0], cfg["selectedColor"][1], cfg["selectedColor"][2]);
                    }
                    if (cfg.contains("unselectedColor")) {
                        modeSelectionTimeAttackTextConfig.unselectedColor = glm::vec3(cfg["unselectedColor"][0], cfg["unselectedColor"][1], cfg["unselectedColor"][2]);
                    }
                    if (cfg.contains("scale")) modeSelectionTimeAttackTextConfig.scale = cfg["scale"];
                }
                
                if (modeSel.contains("pressT")) {
                    auto& cfg = modeSel["pressT"];
                    if (cfg.contains("position")) {
                        auto& pos = cfg["position"];
                        if (pos.contains("offsetX")) modeSelectionPressTConfig.position.offsetX = pos["offsetX"];
                        if (pos.contains("offsetY")) modeSelectionPressTConfig.position.offsetY = pos["offsetY"];
                    }
                    if (cfg.contains("color")) {
                        modeSelectionPressTConfig.color = glm::vec3(cfg["color"][0], cfg["color"][1], cfg["color"][2]);
                    }
                    if (cfg.contains("scale")) modeSelectionPressTConfig.scale = cfg["scale"];
                }
                
                if (modeSel.contains("confirm")) {
                    auto& cfg = modeSel["confirm"];
                    if (cfg.contains("position")) {
                        auto& pos = cfg["position"];
                        if (pos.contains("offsetX")) modeSelectionConfirmConfig.position.offsetX = pos["offsetX"];
                        if (pos.contains("offsetY")) modeSelectionConfirmConfig.position.offsetY = pos["offsetY"];
                    }
                    if (cfg.contains("color")) {
                        modeSelectionConfirmConfig.color = glm::vec3(cfg["color"][0], cfg["color"][1], cfg["color"][2]);
                    }
                    if (cfg.contains("scale")) modeSelectionConfirmConfig.scale = cfg["scale"];
                }
            }
            
            // Tutorial UI
            if (jsonData.contains("tutorial")) {
                auto& tutorial = jsonData["tutorial"];
                
                if (tutorial.contains("stepText")) {
                    auto& cfg = tutorial["stepText"];
                    if (cfg.contains("position")) {
                        auto& pos = cfg["position"];
                        if (pos.contains("offsetX")) tutorialStepTextConfig.position.offsetX = pos["offsetX"];
                        if (pos.contains("offsetY")) tutorialStepTextConfig.position.offsetY = pos["offsetY"];
                    }
                    if (cfg.contains("color")) {
                        tutorialStepTextConfig.color = glm::vec3(cfg["color"][0], cfg["color"][1], cfg["color"][2]);
                    }
                    if (cfg.contains("scale")) tutorialStepTextConfig.scale = cfg["scale"];
                }
                
                if (tutorial.contains("message")) {
                    auto& cfg = tutorial["message"];
                    if (cfg.contains("position")) {
                        auto& pos = cfg["position"];
                        if (pos.contains("offsetX")) tutorialMessageConfig.position.offsetX = pos["offsetX"];
                        if (pos.contains("offsetY")) tutorialMessageConfig.position.offsetY = pos["offsetY"];
                    }
                    if (cfg.contains("completedColor")) {
                        tutorialMessageConfig.completedColor = glm::vec3(cfg["completedColor"][0], cfg["completedColor"][1], cfg["completedColor"][2]);
                    }
                    if (cfg.contains("normalColor")) {
                        tutorialMessageConfig.color = glm::vec3(cfg["normalColor"][0], cfg["normalColor"][1], cfg["normalColor"][2]);
                    }
                    if (cfg.contains("scale")) tutorialMessageConfig.scale = cfg["scale"];
                }
                
                if (tutorial.contains("pressEnter")) {
                    auto& cfg = tutorial["pressEnter"];
                    if (cfg.contains("position")) {
                        auto& pos = cfg["position"];
                        if (pos.contains("offsetX")) tutorialPressEnterConfig.position.offsetX = pos["offsetX"];
                        if (pos.contains("offsetY")) tutorialPressEnterConfig.position.offsetY = pos["offsetY"];
                    }
                    if (cfg.contains("color")) {
                        tutorialPressEnterConfig.color = glm::vec3(cfg["color"][0], cfg["color"][1], cfg["color"][2]);
                    }
                    if (cfg.contains("scale")) tutorialPressEnterConfig.scale = cfg["scale"];
                }
                
                // Tutorial Step-specific UI
                if (tutorial.contains("steps")) {
                    auto& steps = tutorial["steps"];
                    
                    // Step 5
                    if (steps.contains("step5")) {
                        auto& step5 = steps["step5"];
                        if (step5.contains("explainText1")) {
                            auto& cfg = step5["explainText1"];
                            if (cfg.contains("position")) {
                                auto& pos = cfg["position"];
                                if (pos.contains("offsetX")) tutorialStep5ExplainText1Config.position.offsetX = pos["offsetX"];
                                if (pos.contains("offsetY")) tutorialStep5ExplainText1Config.position.offsetY = pos["offsetY"];
                            }
                            if (cfg.contains("color")) {
                                tutorialStep5ExplainText1Config.color = glm::vec3(cfg["color"][0], cfg["color"][1], cfg["color"][2]);
                            }
                            if (cfg.contains("scale")) tutorialStep5ExplainText1Config.scale = cfg["scale"];
                        }
                    }
                    
                    // Step 9
                    if (steps.contains("step9")) {
                        auto& step9 = steps["step9"];
                        if (step9.contains("explainText1")) {
                            auto& cfg = step9["explainText1"];
                            if (cfg.contains("position")) {
                                auto& pos = cfg["position"];
                                if (pos.contains("offsetX")) tutorialStep9ExplainText1Config.position.offsetX = pos["offsetX"];
                                if (pos.contains("offsetY")) tutorialStep9ExplainText1Config.position.offsetY = pos["offsetY"];
                            }
                            if (cfg.contains("color")) {
                                tutorialStep9ExplainText1Config.color = glm::vec3(cfg["color"][0], cfg["color"][1], cfg["color"][2]);
                            }
                            if (cfg.contains("scale")) tutorialStep9ExplainText1Config.scale = cfg["scale"];
                        }
                        if (step9.contains("explainText2")) {
                            auto& cfg = step9["explainText2"];
                            if (cfg.contains("position")) {
                                auto& pos = cfg["position"];
                                if (pos.contains("offsetX")) tutorialStep9ExplainText2Config.position.offsetX = pos["offsetX"];
                                if (pos.contains("offsetY")) tutorialStep9ExplainText2Config.position.offsetY = pos["offsetY"];
                            }
                            if (cfg.contains("color")) {
                                tutorialStep9ExplainText2Config.color = glm::vec3(cfg["color"][0], cfg["color"][1], cfg["color"][2]);
                            }
                            if (cfg.contains("scale")) tutorialStep9ExplainText2Config.scale = cfg["scale"];
                        }
                        if (step9.contains("explainText3")) {
                            auto& cfg = step9["explainText3"];
                            if (cfg.contains("position")) {
                                auto& pos = cfg["position"];
                                if (pos.contains("offsetX")) tutorialStep9ExplainText3Config.position.offsetX = pos["offsetX"];
                                if (pos.contains("offsetY")) tutorialStep9ExplainText3Config.position.offsetY = pos["offsetY"];
                            }
                            if (cfg.contains("color")) {
                                tutorialStep9ExplainText3Config.color = glm::vec3(cfg["color"][0], cfg["color"][1], cfg["color"][2]);
                            }
                            if (cfg.contains("scale")) tutorialStep9ExplainText3Config.scale = cfg["scale"];
                        }
                        if (step9.contains("explainText4")) {
                            auto& cfg = step9["explainText4"];
                            if (cfg.contains("position")) {
                                auto& pos = cfg["position"];
                                if (pos.contains("offsetX")) tutorialStep9ExplainText4Config.position.offsetX = pos["offsetX"];
                                if (pos.contains("offsetY")) tutorialStep9ExplainText4Config.position.offsetY = pos["offsetY"];
                            }
                            if (cfg.contains("color")) {
                                tutorialStep9ExplainText4Config.color = glm::vec3(cfg["color"][0], cfg["color"][1], cfg["color"][2]);
                            }
                            if (cfg.contains("scale")) tutorialStep9ExplainText4Config.scale = cfg["scale"];
                        }
                    }
                    
                    // Step 10
                    if (steps.contains("step10")) {
                        auto& step10 = steps["step10"];
                        if (step10.contains("explainText1")) {
                            auto& cfg = step10["explainText1"];
                            if (cfg.contains("position")) {
                                auto& pos = cfg["position"];
                                if (pos.contains("offsetX")) tutorialStep10ExplainText1Config.position.offsetX = pos["offsetX"];
                                if (pos.contains("offsetY")) tutorialStep10ExplainText1Config.position.offsetY = pos["offsetY"];
                            }
                            if (cfg.contains("color")) {
                                tutorialStep10ExplainText1Config.color = glm::vec3(cfg["color"][0], cfg["color"][1], cfg["color"][2]);
                            }
                            if (cfg.contains("scale")) tutorialStep10ExplainText1Config.scale = cfg["scale"];
                        }
                        if (step10.contains("explainText2")) {
                            auto& cfg = step10["explainText2"];
                            if (cfg.contains("position")) {
                                auto& pos = cfg["position"];
                                if (pos.contains("offsetX")) tutorialStep10ExplainText2Config.position.offsetX = pos["offsetX"];
                                if (pos.contains("offsetY")) tutorialStep10ExplainText2Config.position.offsetY = pos["offsetY"];
                            }
                            if (cfg.contains("color")) {
                                tutorialStep10ExplainText2Config.color = glm::vec3(cfg["color"][0], cfg["color"][1], cfg["color"][2]);
                            }
                            if (cfg.contains("scale")) tutorialStep10ExplainText2Config.scale = cfg["scale"];
                        }
                        if (step10.contains("explainText3")) {
                            auto& cfg = step10["explainText3"];
                            if (cfg.contains("position")) {
                                auto& pos = cfg["position"];
                                if (pos.contains("offsetX")) tutorialStep10ExplainText3Config.position.offsetX = pos["offsetX"];
                                if (pos.contains("offsetY")) tutorialStep10ExplainText3Config.position.offsetY = pos["offsetY"];
                            }
                            if (cfg.contains("color")) {
                                tutorialStep10ExplainText3Config.color = glm::vec3(cfg["color"][0], cfg["color"][1], cfg["color"][2]);
                            }
                            if (cfg.contains("scale")) tutorialStep10ExplainText3Config.scale = cfg["scale"];
                        }
                    }
                }
            }
            
            // Unlock Confirm UI
            if (jsonData.contains("unlockConfirm")) {
                auto& unlock = jsonData["unlockConfirm"];
                
                if (unlock.contains("title")) {
                    auto& cfg = unlock["title"];
                    if (cfg.contains("position")) {
                        auto& pos = cfg["position"];
                        if (pos.contains("offsetX")) unlockConfirmTitleConfig.position.offsetX = pos["offsetX"];
                        if (pos.contains("offsetY")) unlockConfirmTitleConfig.position.offsetY = pos["offsetY"];
                    }
                    if (cfg.contains("color")) {
                        unlockConfirmTitleConfig.color = glm::vec3(cfg["color"][0], cfg["color"][1], cfg["color"][2]);
                    }
                    if (cfg.contains("scale")) unlockConfirmTitleConfig.scale = cfg["scale"];
                }
                
                if (unlock.contains("requiredStars")) {
                    auto& cfg = unlock["requiredStars"];
                    if (cfg.contains("position")) {
                        auto& pos = cfg["position"];
                        if (pos.contains("offsetX")) unlockConfirmRequiredStarsConfig.position.offsetX = pos["offsetX"];
                        if (pos.contains("offsetY")) unlockConfirmRequiredStarsConfig.position.offsetY = pos["offsetY"];
                    }
                    if (cfg.contains("color")) {
                        unlockConfirmRequiredStarsConfig.color = glm::vec3(cfg["color"][0], cfg["color"][1], cfg["color"][2]);
                    }
                    if (cfg.contains("scale")) unlockConfirmRequiredStarsConfig.scale = cfg["scale"];
                }
                
                if (unlock.contains("unlockButton")) {
                    auto& cfg = unlock["unlockButton"];
                    if (cfg.contains("position")) {
                        auto& pos = cfg["position"];
                        if (pos.contains("offsetX")) unlockConfirmUnlockButtonConfig.position.offsetX = pos["offsetX"];
                        if (pos.contains("offsetY")) unlockConfirmUnlockButtonConfig.position.offsetY = pos["offsetY"];
                    }
                    if (cfg.contains("color")) {
                        unlockConfirmUnlockButtonConfig.color = glm::vec3(cfg["color"][0], cfg["color"][1], cfg["color"][2]);
                    }
                    if (cfg.contains("scale")) unlockConfirmUnlockButtonConfig.scale = cfg["scale"];
                }
                
                if (unlock.contains("cancelButton")) {
                    auto& cfg = unlock["cancelButton"];
                    if (cfg.contains("position")) {
                        auto& pos = cfg["position"];
                        if (pos.contains("offsetX")) unlockConfirmCancelButtonConfig.position.offsetX = pos["offsetX"];
                        if (pos.contains("offsetY")) unlockConfirmCancelButtonConfig.position.offsetY = pos["offsetY"];
                    }
                    if (cfg.contains("color")) {
                        unlockConfirmCancelButtonConfig.color = glm::vec3(cfg["color"][0], cfg["color"][1], cfg["color"][2]);
                    }
                    if (cfg.contains("scale")) unlockConfirmCancelButtonConfig.scale = cfg["scale"];
                }
            }
            
            // Star Insufficient UI
            if (jsonData.contains("starInsufficient")) {
                auto& starIns = jsonData["starInsufficient"];
                
                if (starIns.contains("title")) {
                    auto& cfg = starIns["title"];
                    if (cfg.contains("position")) {
                        auto& pos = cfg["position"];
                        if (pos.contains("offsetX")) starInsufficientTitleConfig.position.offsetX = pos["offsetX"];
                        if (pos.contains("offsetY")) starInsufficientTitleConfig.position.offsetY = pos["offsetY"];
                    }
                    if (cfg.contains("color")) {
                        starInsufficientTitleConfig.color = glm::vec3(cfg["color"][0], cfg["color"][1], cfg["color"][2]);
                    }
                    if (cfg.contains("scale")) starInsufficientTitleConfig.scale = cfg["scale"];
                }
                
                if (starIns.contains("requiredStars")) {
                    auto& cfg = starIns["requiredStars"];
                    if (cfg.contains("position")) {
                        auto& pos = cfg["position"];
                        if (pos.contains("offsetX")) starInsufficientRequiredStarsConfig.position.offsetX = pos["offsetX"];
                        if (pos.contains("offsetY")) starInsufficientRequiredStarsConfig.position.offsetY = pos["offsetY"];
                    }
                    if (cfg.contains("color")) {
                        starInsufficientRequiredStarsConfig.color = glm::vec3(cfg["color"][0], cfg["color"][1], cfg["color"][2]);
                    }
                    if (cfg.contains("scale")) starInsufficientRequiredStarsConfig.scale = cfg["scale"];
                }
                
                if (starIns.contains("collectStars")) {
                    auto& cfg = starIns["collectStars"];
                    if (cfg.contains("position")) {
                        auto& pos = cfg["position"];
                        if (pos.contains("offsetX")) starInsufficientCollectStarsConfig.position.offsetX = pos["offsetX"];
                        if (pos.contains("offsetY")) starInsufficientCollectStarsConfig.position.offsetY = pos["offsetY"];
                    }
                    if (cfg.contains("color")) {
                        starInsufficientCollectStarsConfig.color = glm::vec3(cfg["color"][0], cfg["color"][1], cfg["color"][2]);
                    }
                    if (cfg.contains("scale")) starInsufficientCollectStarsConfig.scale = cfg["scale"];
                }
                
                if (starIns.contains("okButton")) {
                    auto& cfg = starIns["okButton"];
                    if (cfg.contains("position")) {
                        auto& pos = cfg["position"];
                        if (pos.contains("offsetX")) starInsufficientOkButtonConfig.position.offsetX = pos["offsetX"];
                        if (pos.contains("offsetY")) starInsufficientOkButtonConfig.position.offsetY = pos["offsetY"];
                    }
                    if (cfg.contains("color")) {
                        starInsufficientOkButtonConfig.color = glm::vec3(cfg["color"][0], cfg["color"][1], cfg["color"][2]);
                    }
                    if (cfg.contains("scale")) starInsufficientOkButtonConfig.scale = cfg["scale"];
                }
            }
            
            // Ending UI
            if (jsonData.contains("ending")) {
                auto& ending = jsonData["ending"];
                
                if (ending.contains("thankYou")) {
                    auto& cfg = ending["thankYou"];
                    if (cfg.contains("position")) {
                        auto& pos = cfg["position"];
                        if (pos.contains("offsetX")) endingThankYouConfig.position.offsetX = pos["offsetX"];
                        if (pos.contains("offsetY")) endingThankYouConfig.position.offsetY = pos["offsetY"];
                    }
                    if (cfg.contains("color")) {
                        endingThankYouConfig.color = glm::vec3(cfg["color"][0], cfg["color"][1], cfg["color"][2]);
                    }
                    if (cfg.contains("scale")) endingThankYouConfig.scale = cfg["scale"];
                }
                
                if (ending.contains("congratulations")) {
                    auto& cfg = ending["congratulations"];
                    if (cfg.contains("position")) {
                        auto& pos = cfg["position"];
                        if (pos.contains("offsetX")) endingCongratulationsConfig.position.offsetX = pos["offsetX"];
                        if (pos.contains("offsetY")) endingCongratulationsConfig.position.offsetY = pos["offsetY"];
                    }
                    if (cfg.contains("color")) {
                        endingCongratulationsConfig.color = glm::vec3(cfg["color"][0], cfg["color"][1], cfg["color"][2]);
                    }
                    if (cfg.contains("scale")) endingCongratulationsConfig.scale = cfg["scale"];
                }
                
                if (ending.contains("seeYouAgain")) {
                    auto& cfg = ending["seeYouAgain"];
                    if (cfg.contains("position")) {
                        auto& pos = cfg["position"];
                        if (pos.contains("offsetX")) endingSeeYouAgainConfig.position.offsetX = pos["offsetX"];
                        if (pos.contains("offsetY")) endingSeeYouAgainConfig.position.offsetY = pos["offsetY"];
                    }
                    if (cfg.contains("color")) {
                        endingSeeYouAgainConfig.color = glm::vec3(cfg["color"][0], cfg["color"][1], cfg["color"][2]);
                    }
                    if (cfg.contains("scale")) endingSeeYouAgainConfig.scale = cfg["scale"];
                }
                
                if (ending.contains("skip")) {
                    auto& cfg = ending["skip"];
                    if (cfg.contains("position")) {
                        auto& pos = cfg["position"];
                        if (pos.contains("offsetX")) endingSkipConfig.position.offsetX = pos["offsetX"];
                        if (pos.contains("offsetY")) endingSkipConfig.position.offsetY = pos["offsetY"];
                        if (pos.contains("useRelative")) endingSkipConfig.position.useRelative = pos["useRelative"];
                        if (pos.contains("absoluteX")) endingSkipConfig.position.absoluteX = pos["absoluteX"];
                        if (pos.contains("absoluteY")) endingSkipConfig.position.absoluteY = pos["absoluteY"];
                    }
                    if (cfg.contains("color")) {
                        endingSkipConfig.color = glm::vec3(cfg["color"][0], cfg["color"][1], cfg["color"][2]);
                    }
                    if (cfg.contains("scale")) endingSkipConfig.scale = cfg["scale"];
                }
            }
            
            // Warp Tutorial UI
            if (jsonData.contains("warpTutorial")) {
                auto& warp = jsonData["warpTutorial"];
                
                if (warp.contains("title")) {
                    auto& cfg = warp["title"];
                    if (cfg.contains("position")) {
                        auto& pos = cfg["position"];
                        if (pos.contains("offsetX")) warpTutorialTitleConfig.position.offsetX = pos["offsetX"];
                        if (pos.contains("offsetY")) warpTutorialTitleConfig.position.offsetY = pos["offsetY"];
                    }
                    if (cfg.contains("color")) {
                        warpTutorialTitleConfig.color = glm::vec3(cfg["color"][0], cfg["color"][1], cfg["color"][2]);
                    }
                    if (cfg.contains("scale")) warpTutorialTitleConfig.scale = cfg["scale"];
                }
                
                if (warp.contains("description1")) {
                    auto& cfg = warp["description1"];
                    if (cfg.contains("position")) {
                        auto& pos = cfg["position"];
                        if (pos.contains("offsetX")) warpTutorialDescription1Config.position.offsetX = pos["offsetX"];
                        if (pos.contains("offsetY")) warpTutorialDescription1Config.position.offsetY = pos["offsetY"];
                    }
                    if (cfg.contains("color")) {
                        warpTutorialDescription1Config.color = glm::vec3(cfg["color"][0], cfg["color"][1], cfg["color"][2]);
                    }
                    if (cfg.contains("scale")) warpTutorialDescription1Config.scale = cfg["scale"];
                }
                
                if (warp.contains("description2")) {
                    auto& cfg = warp["description2"];
                    if (cfg.contains("position")) {
                        auto& pos = cfg["position"];
                        if (pos.contains("offsetX")) warpTutorialDescription2Config.position.offsetX = pos["offsetX"];
                        if (pos.contains("offsetY")) warpTutorialDescription2Config.position.offsetY = pos["offsetY"];
                    }
                    if (cfg.contains("color")) {
                        warpTutorialDescription2Config.color = glm::vec3(cfg["color"][0], cfg["color"][1], cfg["color"][2]);
                    }
                    if (cfg.contains("scale")) warpTutorialDescription2Config.scale = cfg["scale"];
                }
                
                if (warp.contains("enterButton")) {
                    auto& cfg = warp["enterButton"];
                    if (cfg.contains("position")) {
                        auto& pos = cfg["position"];
                        if (pos.contains("offsetX")) warpTutorialEnterButtonConfig.position.offsetX = pos["offsetX"];
                        if (pos.contains("offsetY")) warpTutorialEnterButtonConfig.position.offsetY = pos["offsetY"];
                    }
                    if (cfg.contains("color")) {
                        warpTutorialEnterButtonConfig.color = glm::vec3(cfg["color"][0], cfg["color"][1], cfg["color"][2]);
                    }
                    if (cfg.contains("scale")) warpTutorialEnterButtonConfig.scale = cfg["scale"];
                }
            }
            
            // Stage 0 Tutorial UI
            if (jsonData.contains("stage0Tutorial")) {
                auto& stage0 = jsonData["stage0Tutorial"];
                
                if (stage0.contains("line1")) {
                    auto& cfg = stage0["line1"];
                    if (cfg.contains("position")) {
                        auto& pos = cfg["position"];
                        if (pos.contains("offsetX")) stage0TutorialLine1Config.position.offsetX = pos["offsetX"];
                        if (pos.contains("offsetY")) stage0TutorialLine1Config.position.offsetY = pos["offsetY"];
                    }
                    if (cfg.contains("color")) {
                        stage0TutorialLine1Config.color = glm::vec3(cfg["color"][0], cfg["color"][1], cfg["color"][2]);
                    }
                    if (cfg.contains("scale")) stage0TutorialLine1Config.scale = cfg["scale"];
                }
                
                if (stage0.contains("line2")) {
                    auto& cfg = stage0["line2"];
                    if (cfg.contains("position")) {
                        auto& pos = cfg["position"];
                        if (pos.contains("offsetX")) stage0TutorialLine2Config.position.offsetX = pos["offsetX"];
                        if (pos.contains("offsetY")) stage0TutorialLine2Config.position.offsetY = pos["offsetY"];
                    }
                    if (cfg.contains("color")) {
                        stage0TutorialLine2Config.color = glm::vec3(cfg["color"][0], cfg["color"][1], cfg["color"][2]);
                    }
                    if (cfg.contains("scale")) stage0TutorialLine2Config.scale = cfg["scale"];
                }
                
                if (stage0.contains("line3")) {
                    auto& cfg = stage0["line3"];
                    if (cfg.contains("position")) {
                        auto& pos = cfg["position"];
                        if (pos.contains("offsetX")) stage0TutorialLine3Config.position.offsetX = pos["offsetX"];
                        if (pos.contains("offsetY")) stage0TutorialLine3Config.position.offsetY = pos["offsetY"];
                    }
                    if (cfg.contains("color")) {
                        stage0TutorialLine3Config.color = glm::vec3(cfg["color"][0], cfg["color"][1], cfg["color"][2]);
                    }
                    if (cfg.contains("scale")) stage0TutorialLine3Config.scale = cfg["scale"];
                }
                
                if (stage0.contains("line4")) {
                    auto& cfg = stage0["line4"];
                    if (cfg.contains("position")) {
                        auto& pos = cfg["position"];
                        if (pos.contains("offsetX")) stage0TutorialLine4Config.position.offsetX = pos["offsetX"];
                        if (pos.contains("offsetY")) stage0TutorialLine4Config.position.offsetY = pos["offsetY"];
                    }
                    if (cfg.contains("color")) {
                        stage0TutorialLine4Config.color = glm::vec3(cfg["color"][0], cfg["color"][1], cfg["color"][2]);
                    }
                    if (cfg.contains("scale")) stage0TutorialLine4Config.scale = cfg["scale"];
                }
                
                if (stage0.contains("line5")) {
                    auto& cfg = stage0["line5"];
                    if (cfg.contains("position")) {
                        auto& pos = cfg["position"];
                        if (pos.contains("offsetX")) stage0TutorialLine5Config.position.offsetX = pos["offsetX"];
                        if (pos.contains("offsetY")) stage0TutorialLine5Config.position.offsetY = pos["offsetY"];
                    }
                    if (cfg.contains("color")) {
                        stage0TutorialLine5Config.color = glm::vec3(cfg["color"][0], cfg["color"][1], cfg["color"][2]);
                    }
                    if (cfg.contains("scale")) stage0TutorialLine5Config.scale = cfg["scale"];
                }
                
                if (stage0.contains("okButton")) {
                    auto& cfg = stage0["okButton"];
                    if (cfg.contains("position")) {
                        auto& pos = cfg["position"];
                        if (pos.contains("offsetX")) stage0TutorialOkButtonConfig.position.offsetX = pos["offsetX"];
                        if (pos.contains("offsetY")) stage0TutorialOkButtonConfig.position.offsetY = pos["offsetY"];
                    }
                    if (cfg.contains("color")) {
                        stage0TutorialOkButtonConfig.color = glm::vec3(cfg["color"][0], cfg["color"][1], cfg["color"][2]);
                    }
                    if (cfg.contains("scale")) stage0TutorialOkButtonConfig.scale = cfg["scale"];
                }
            }
            
            // Easy Mode Explanation UI
            if (jsonData.contains("easyModeExplanation")) {
                auto& easy = jsonData["easyModeExplanation"];
                
                if (easy.contains("title")) {
                    auto& cfg = easy["title"];
                    if (cfg.contains("position")) {
                        auto& pos = cfg["position"];
                        if (pos.contains("offsetX")) easyModeExplanationTitleConfig.position.offsetX = pos["offsetX"];
                        if (pos.contains("offsetY")) easyModeExplanationTitleConfig.position.offsetY = pos["offsetY"];
                    }
                    if (cfg.contains("color")) {
                        easyModeExplanationTitleConfig.color = glm::vec3(cfg["color"][0], cfg["color"][1], cfg["color"][2]);
                    }
                    if (cfg.contains("scale")) easyModeExplanationTitleConfig.scale = cfg["scale"];
                }
                
                if (easy.contains("description1")) {
                    auto& cfg = easy["description1"];
                    if (cfg.contains("position")) {
                        auto& pos = cfg["position"];
                        if (pos.contains("offsetX")) easyModeExplanationDescription1Config.position.offsetX = pos["offsetX"];
                        if (pos.contains("offsetY")) easyModeExplanationDescription1Config.position.offsetY = pos["offsetY"];
                    }
                    if (cfg.contains("color")) {
                        easyModeExplanationDescription1Config.color = glm::vec3(cfg["color"][0], cfg["color"][1], cfg["color"][2]);
                    }
                    if (cfg.contains("scale")) easyModeExplanationDescription1Config.scale = cfg["scale"];
                }
                
                if (easy.contains("description2")) {
                    auto& cfg = easy["description2"];
                    if (cfg.contains("position")) {
                        auto& pos = cfg["position"];
                        if (pos.contains("offsetX")) easyModeExplanationDescription2Config.position.offsetX = pos["offsetX"];
                        if (pos.contains("offsetY")) easyModeExplanationDescription2Config.position.offsetY = pos["offsetY"];
                    }
                    if (cfg.contains("color")) {
                        easyModeExplanationDescription2Config.color = glm::vec3(cfg["color"][0], cfg["color"][1], cfg["color"][2]);
                    }
                    if (cfg.contains("scale")) easyModeExplanationDescription2Config.scale = cfg["scale"];
                }
                
                if (easy.contains("item1")) {
                    auto& cfg = easy["item1"];
                    if (cfg.contains("position")) {
                        auto& pos = cfg["position"];
                        if (pos.contains("offsetX")) easyModeExplanationItem1Config.position.offsetX = pos["offsetX"];
                        if (pos.contains("offsetY")) easyModeExplanationItem1Config.position.offsetY = pos["offsetY"];
                    }
                    if (cfg.contains("color")) {
                        easyModeExplanationItem1Config.color = glm::vec3(cfg["color"][0], cfg["color"][1], cfg["color"][2]);
                    }
                    if (cfg.contains("scale")) easyModeExplanationItem1Config.scale = cfg["scale"];
                }
                
                if (easy.contains("item2")) {
                    auto& cfg = easy["item2"];
                    if (cfg.contains("position")) {
                        auto& pos = cfg["position"];
                        if (pos.contains("offsetX")) easyModeExplanationItem2Config.position.offsetX = pos["offsetX"];
                        if (pos.contains("offsetY")) easyModeExplanationItem2Config.position.offsetY = pos["offsetY"];
                    }
                    if (cfg.contains("color")) {
                        easyModeExplanationItem2Config.color = glm::vec3(cfg["color"][0], cfg["color"][1], cfg["color"][2]);
                    }
                    if (cfg.contains("scale")) easyModeExplanationItem2Config.scale = cfg["scale"];
                }
                
                if (easy.contains("item3")) {
                    auto& cfg = easy["item3"];
                    if (cfg.contains("position")) {
                        auto& pos = cfg["position"];
                        if (pos.contains("offsetX")) easyModeExplanationItem3Config.position.offsetX = pos["offsetX"];
                        if (pos.contains("offsetY")) easyModeExplanationItem3Config.position.offsetY = pos["offsetY"];
                    }
                    if (cfg.contains("color")) {
                        easyModeExplanationItem3Config.color = glm::vec3(cfg["color"][0], cfg["color"][1], cfg["color"][2]);
                    }
                    if (cfg.contains("scale")) easyModeExplanationItem3Config.scale = cfg["scale"];
                }
                
                if (easy.contains("item4a")) {
                    auto& cfg = easy["item4a"];
                    if (cfg.contains("position")) {
                        auto& pos = cfg["position"];
                        if (pos.contains("offsetX")) easyModeExplanationItem4aConfig.position.offsetX = pos["offsetX"];
                        if (pos.contains("offsetY")) easyModeExplanationItem4aConfig.position.offsetY = pos["offsetY"];
                    }
                    if (cfg.contains("color")) {
                        easyModeExplanationItem4aConfig.color = glm::vec3(cfg["color"][0], cfg["color"][1], cfg["color"][2]);
                    }
                    if (cfg.contains("scale")) easyModeExplanationItem4aConfig.scale = cfg["scale"];
                }
                
                if (easy.contains("item4b")) {
                    auto& cfg = easy["item4b"];
                    if (cfg.contains("position")) {
                        auto& pos = cfg["position"];
                        if (pos.contains("offsetX")) easyModeExplanationItem4bConfig.position.offsetX = pos["offsetX"];
                        if (pos.contains("offsetY")) easyModeExplanationItem4bConfig.position.offsetY = pos["offsetY"];
                    }
                    if (cfg.contains("color")) {
                        easyModeExplanationItem4bConfig.color = glm::vec3(cfg["color"][0], cfg["color"][1], cfg["color"][2]);
                    }
                    if (cfg.contains("scale")) easyModeExplanationItem4bConfig.scale = cfg["scale"];
                }
                
                if (easy.contains("okButton")) {
                    auto& cfg = easy["okButton"];
                    if (cfg.contains("position")) {
                        auto& pos = cfg["position"];
                        if (pos.contains("offsetX")) easyModeExplanationOkButtonConfig.position.offsetX = pos["offsetX"];
                        if (pos.contains("offsetY")) easyModeExplanationOkButtonConfig.position.offsetY = pos["offsetY"];
                    }
                    if (cfg.contains("color")) {
                        easyModeExplanationOkButtonConfig.color = glm::vec3(cfg["color"][0], cfg["color"][1], cfg["color"][2]);
                    }
                    if (cfg.contains("scale")) easyModeExplanationOkButtonConfig.scale = cfg["scale"];
                }
            }
            
            // Countdown UI
            if (jsonData.contains("countdown")) {
                auto& countdown = jsonData["countdown"];
                
                if (countdown.contains("number")) {
                    auto& cfg = countdown["number"];
                    if (cfg.contains("position")) {
                        auto& pos = cfg["position"];
                        if (pos.contains("offsetX")) countdownNumberConfig.position.offsetX = pos["offsetX"];
                        if (pos.contains("offsetY")) countdownNumberConfig.position.offsetY = pos["offsetY"];
                    }
                    if (cfg.contains("color")) {
                        countdownNumberConfig.color = glm::vec3(cfg["color"][0], cfg["color"][1], cfg["color"][2]);
                    }
                    if (cfg.contains("scale")) countdownNumberConfig.scale = cfg["scale"];
                }
            }
            
            // Stage Selection Assist UI
            if (jsonData.contains("stageSelectionAssist")) {
                auto& assist = jsonData["stageSelectionAssist"];
                
                if (assist.contains("text")) {
                    auto& cfg = assist["text"];
                    if (cfg.contains("position")) {
                        auto& pos = cfg["position"];
                        if (pos.contains("offsetX")) stageSelectionAssistTextConfig.position.offsetX = pos["offsetX"];
                        if (pos.contains("offsetY")) stageSelectionAssistTextConfig.position.offsetY = pos["offsetY"];
                    }
                    if (cfg.contains("color")) {
                        stageSelectionAssistTextConfig.color = glm::vec3(cfg["color"][0], cfg["color"][1], cfg["color"][2]);
                    }
                    if (cfg.contains("scale")) stageSelectionAssistTextConfig.scale = cfg["scale"];
                }
            }
            
            // Staff Roll UI
            if (jsonData.contains("staffRoll")) {
                auto& staff = jsonData["staffRoll"];
                
                if (staff.contains("skip")) {
                    auto& cfg = staff["skip"];
                    if (cfg.contains("position")) {
                        auto& pos = cfg["position"];
                        if (pos.contains("offsetX")) staffRollSkipConfig.position.offsetX = pos["offsetX"];
                        if (pos.contains("offsetY")) staffRollSkipConfig.position.offsetY = pos["offsetY"];
                    }
                    if (cfg.contains("color")) {
                        staffRollSkipConfig.color = glm::vec3(cfg["color"][0], cfg["color"][1], cfg["color"][2]);
                    }
                    if (cfg.contains("scale")) staffRollSkipConfig.scale = cfg["scale"];
                }
                
                if (staff.contains("title")) {
                    auto& cfg = staff["title"];
                    if (cfg.contains("position")) {
                        auto& pos = cfg["position"];
                        if (pos.contains("offsetX")) staffRollTitleConfig.position.offsetX = pos["offsetX"];
                        if (pos.contains("offsetY")) staffRollTitleConfig.position.offsetY = pos["offsetY"];
                    }
                    if (cfg.contains("color")) {
                        staffRollTitleConfig.color = glm::vec3(cfg["color"][0], cfg["color"][1], cfg["color"][2]);
                    }
                    if (cfg.contains("scale")) staffRollTitleConfig.scale = cfg["scale"];
                }
                
                if (staff.contains("role")) {
                    auto& cfg = staff["role"];
                    if (cfg.contains("position")) {
                        auto& pos = cfg["position"];
                        if (pos.contains("offsetX")) staffRollRoleConfig.position.offsetX = pos["offsetX"];
                        if (pos.contains("offsetY")) staffRollRoleConfig.position.offsetY = pos["offsetY"];
                    }
                    if (cfg.contains("color")) {
                        staffRollRoleConfig.color = glm::vec3(cfg["color"][0], cfg["color"][1], cfg["color"][2]);
                    }
                    if (cfg.contains("scale")) staffRollRoleConfig.scale = cfg["scale"];
                }
                
                if (staff.contains("name")) {
                    auto& cfg = staff["name"];
                    if (cfg.contains("position")) {
                        auto& pos = cfg["position"];
                        if (pos.contains("offsetX")) staffRollNameConfig.position.offsetX = pos["offsetX"];
                        if (pos.contains("offsetY")) staffRollNameConfig.position.offsetY = pos["offsetY"];
                    }
                    if (cfg.contains("color")) {
                        staffRollNameConfig.color = glm::vec3(cfg["color"][0], cfg["color"][1], cfg["color"][2]);
                    }
                    if (cfg.contains("scale")) staffRollNameConfig.scale = cfg["scale"];
                }
                
                if (staff.contains("spacing")) {
                    staffRollSpacing = staff["spacing"];
                }
            }
            
            // Game UI
            if (jsonData.contains("gameUI")) {
                auto& gameUI = jsonData["gameUI"];
                
                if (gameUI.contains("timeDisplay")) {
                    auto& cfg = gameUI["timeDisplay"];
                    if (cfg.contains("position")) {
                        auto& pos = cfg["position"];
                        if (pos.contains("absoluteX")) gameUITimeDisplayConfig.position.absoluteX = pos["absoluteX"];
                        if (pos.contains("absoluteY")) gameUITimeDisplayConfig.position.absoluteY = pos["absoluteY"];
                        if (pos.contains("useRelative")) gameUITimeDisplayConfig.position.useRelative = pos["useRelative"];
                    }
                    if (cfg.contains("normalColor")) {
                        gameUITimeDisplayConfig.normalColor = glm::vec3(cfg["normalColor"][0], cfg["normalColor"][1], cfg["normalColor"][2]);
                    }
                    if (cfg.contains("warningColor")) {
                        gameUITimeDisplayConfig.warningColor = glm::vec3(cfg["warningColor"][0], cfg["warningColor"][1], cfg["warningColor"][2]);
                    }
                    if (cfg.contains("scale")) gameUITimeDisplayConfig.scale = cfg["scale"];
                }
                
                if (gameUI.contains("timeAttackDisplay")) {
                    auto& cfg = gameUI["timeAttackDisplay"];
                    if (cfg.contains("position")) {
                        auto& pos = cfg["position"];
                        if (pos.contains("absoluteX")) gameUITimeAttackDisplayConfig.position.absoluteX = pos["absoluteX"];
                        if (pos.contains("absoluteY")) gameUITimeAttackDisplayConfig.position.absoluteY = pos["absoluteY"];
                        if (pos.contains("useRelative")) gameUITimeAttackDisplayConfig.position.useRelative = pos["useRelative"];
                    }
                    if (cfg.contains("color")) {
                        gameUITimeAttackDisplayConfig.color = glm::vec3(cfg["color"][0], cfg["color"][1], cfg["color"][2]);
                    }
                    if (cfg.contains("scale")) gameUITimeAttackDisplayConfig.scale = cfg["scale"];
                }
                
                if (gameUI.contains("bestTime")) {
                    auto& cfg = gameUI["bestTime"];
                    if (cfg.contains("position")) {
                        auto& pos = cfg["position"];
                        if (pos.contains("absoluteX")) gameUIBestTimeConfig.position.absoluteX = pos["absoluteX"];
                        if (pos.contains("absoluteY")) gameUIBestTimeConfig.position.absoluteY = pos["absoluteY"];
                        if (pos.contains("useRelative")) gameUIBestTimeConfig.position.useRelative = pos["useRelative"];
                    }
                    if (cfg.contains("color")) {
                        gameUIBestTimeConfig.color = glm::vec3(cfg["color"][0], cfg["color"][1], cfg["color"][2]);
                    }
                    if (cfg.contains("scale")) gameUIBestTimeConfig.scale = cfg["scale"];
                }
                
                if (gameUI.contains("goalDisplay")) {
                    auto& cfg = gameUI["goalDisplay"];
                    if (cfg.contains("position")) {
                        auto& pos = cfg["position"];
                        if (pos.contains("absoluteX")) gameUIGoalDisplayConfig.position.absoluteX = pos["absoluteX"];
                        if (pos.contains("absoluteY")) gameUIGoalDisplayConfig.position.absoluteY = pos["absoluteY"];
                        if (pos.contains("useRelative")) gameUIGoalDisplayConfig.position.useRelative = pos["useRelative"];
                    }
                    if (cfg.contains("color")) {
                        gameUIGoalDisplayConfig.color = glm::vec3(cfg["color"][0], cfg["color"][1], cfg["color"][2]);
                    }
                    if (cfg.contains("scale")) gameUIGoalDisplayConfig.scale = cfg["scale"];
                }
                
                if (gameUI.contains("goalTime5s")) {
                    auto& cfg = gameUI["goalTime5s"];
                    if (cfg.contains("position")) {
                        auto& pos = cfg["position"];
                        if (pos.contains("absoluteX")) gameUIGoalTime5sConfig.position.absoluteX = pos["absoluteX"];
                        if (pos.contains("absoluteY")) gameUIGoalTime5sConfig.position.absoluteY = pos["absoluteY"];
                        if (pos.contains("useRelative")) gameUIGoalTime5sConfig.position.useRelative = pos["useRelative"];
                    }
                    if (cfg.contains("color")) {
                        gameUIGoalTime5sConfig.color = glm::vec3(cfg["color"][0], cfg["color"][1], cfg["color"][2]);
                    }
                    if (cfg.contains("scale")) gameUIGoalTime5sConfig.scale = cfg["scale"];
                }
                
                if (gameUI.contains("goalTime10s")) {
                    auto& cfg = gameUI["goalTime10s"];
                    if (cfg.contains("position")) {
                        auto& pos = cfg["position"];
                        if (pos.contains("absoluteX")) gameUIGoalTime10sConfig.position.absoluteX = pos["absoluteX"];
                        if (pos.contains("absoluteY")) gameUIGoalTime10sConfig.position.absoluteY = pos["absoluteY"];
                        if (pos.contains("useRelative")) gameUIGoalTime10sConfig.position.useRelative = pos["useRelative"];
                    }
                    if (cfg.contains("color")) {
                        gameUIGoalTime10sConfig.color = glm::vec3(cfg["color"][0], cfg["color"][1], cfg["color"][2]);
                    }
                    if (cfg.contains("scale")) gameUIGoalTime10sConfig.scale = cfg["scale"];
                }
                
                if (gameUI.contains("goalTime20s")) {
                    auto& cfg = gameUI["goalTime20s"];
                    if (cfg.contains("position")) {
                        auto& pos = cfg["position"];
                        if (pos.contains("absoluteX")) gameUIGoalTime20sConfig.position.absoluteX = pos["absoluteX"];
                        if (pos.contains("absoluteY")) gameUIGoalTime20sConfig.position.absoluteY = pos["absoluteY"];
                        if (pos.contains("useRelative")) gameUIGoalTime20sConfig.position.useRelative = pos["useRelative"];
                    }
                    if (cfg.contains("color")) {
                        gameUIGoalTime20sConfig.color = glm::vec3(cfg["color"][0], cfg["color"][1], cfg["color"][2]);
                    }
                    if (cfg.contains("scale")) gameUIGoalTime20sConfig.scale = cfg["scale"];
                }
                
                if (gameUI.contains("stars")) {
                    auto& cfg = gameUI["stars"];
                    if (cfg.contains("position")) {
                        auto& pos = cfg["position"];
                        if (pos.contains("absoluteX")) gameUIStarsConfig.position.absoluteX = pos["absoluteX"];
                        if (pos.contains("absoluteY")) gameUIStarsConfig.position.absoluteY = pos["absoluteY"];
                        if (pos.contains("useRelative")) gameUIStarsConfig.position.useRelative = pos["useRelative"];
                        if (pos.contains("spacing")) gameUIStarsConfig.spacing = pos["spacing"];
                    }
                    if (cfg.contains("activeColor")) {
                        gameUIStarsConfig.selectedColor = glm::vec3(cfg["activeColor"][0], cfg["activeColor"][1], cfg["activeColor"][2]);
                    }
                    if (cfg.contains("inactiveColor")) {
                        gameUIStarsConfig.unselectedColor = glm::vec3(cfg["inactiveColor"][0], cfg["inactiveColor"][1], cfg["inactiveColor"][2]);
                    }
                    if (cfg.contains("scale")) gameUIStarsConfig.scale = cfg["scale"];
                }
                
                if (gameUI.contains("hearts")) {
                    auto& cfg = gameUI["hearts"];
                    if (cfg.contains("position")) {
                        auto& pos = cfg["position"];
                        if (pos.contains("absoluteX")) gameUIHeartsConfig.position.absoluteX = pos["absoluteX"];
                        if (pos.contains("absoluteY")) gameUIHeartsConfig.position.absoluteY = pos["absoluteY"];
                        if (pos.contains("useRelative")) gameUIHeartsConfig.position.useRelative = pos["useRelative"];
                        if (pos.contains("spacing")) gameUIHeartsConfig.spacing = pos["spacing"];
                    }
                    if (cfg.contains("activeColor")) {
                        gameUIHeartsConfig.selectedColor = glm::vec3(cfg["activeColor"][0], cfg["activeColor"][1], cfg["activeColor"][2]);
                    }
                    if (cfg.contains("inactiveColor")) {
                        gameUIHeartsConfig.unselectedColor = glm::vec3(cfg["inactiveColor"][0], cfg["inactiveColor"][1], cfg["inactiveColor"][2]);
                    }
                    if (cfg.contains("scale")) gameUIHeartsConfig.scale = cfg["scale"];
                }
                
                if (gameUI.contains("skills")) {
                    auto& skills = gameUI["skills"];
                    
                    // timeStop
                    if (skills.contains("timeStop")) {
                        auto& cfg = skills["timeStop"];
                        if (cfg.contains("position")) {
                            auto& pos = cfg["position"];
                            if (pos.contains("absoluteX")) gameUITimeStopSkillConfig.position.absoluteX = pos["absoluteX"];
                            if (pos.contains("absoluteY")) gameUITimeStopSkillConfig.position.absoluteY = pos["absoluteY"];
                            if (pos.contains("useRelative")) gameUITimeStopSkillConfig.position.useRelative = pos["useRelative"];
                        }
                        if (cfg.contains("countOffset")) gameUITimeStopSkillConfig.countOffset = cfg["countOffset"];
                        if (cfg.contains("instructionOffset")) gameUITimeStopSkillConfig.instructionOffset = cfg["instructionOffset"];
                        if (cfg.contains("color")) {
                            gameUITimeStopSkillConfig.color = glm::vec3(cfg["color"][0], cfg["color"][1], cfg["color"][2]);
                        }
                        if (cfg.contains("disabledColor")) {
                            gameUITimeStopSkillConfig.disabledColor = glm::vec3(cfg["disabledColor"][0], cfg["disabledColor"][1], cfg["disabledColor"][2]);
                        }
                        if (cfg.contains("activeColor")) {
                            gameUITimeStopSkillConfig.activeColor = glm::vec3(cfg["activeColor"][0], cfg["activeColor"][1], cfg["activeColor"][2]);
                        }
                        if (cfg.contains("scale")) gameUITimeStopSkillConfig.scale = cfg["scale"];
                        if (cfg.contains("countScale")) gameUITimeStopSkillConfig.countScale = cfg["countScale"];
                        if (cfg.contains("instructionScale")) gameUITimeStopSkillConfig.instructionScale = cfg["instructionScale"];
                    }
                    
                    // doubleJump
                    if (skills.contains("doubleJump")) {
                        auto& cfg = skills["doubleJump"];
                        if (cfg.contains("position")) {
                            auto& pos = cfg["position"];
                            if (pos.contains("absoluteX")) gameUIDoubleJumpSkillConfig.position.absoluteX = pos["absoluteX"];
                            if (pos.contains("absoluteY")) gameUIDoubleJumpSkillConfig.position.absoluteY = pos["absoluteY"];
                            if (pos.contains("useRelative")) gameUIDoubleJumpSkillConfig.position.useRelative = pos["useRelative"];
                        }
                        if (cfg.contains("countOffset")) gameUIDoubleJumpSkillConfig.countOffset = cfg["countOffset"];
                        if (cfg.contains("instructionOffset")) gameUIDoubleJumpSkillConfig.instructionOffset = cfg["instructionOffset"];
                        if (cfg.contains("color")) {
                            gameUIDoubleJumpSkillConfig.color = glm::vec3(cfg["color"][0], cfg["color"][1], cfg["color"][2]);
                        }
                        if (cfg.contains("disabledColor")) {
                            gameUIDoubleJumpSkillConfig.disabledColor = glm::vec3(cfg["disabledColor"][0], cfg["disabledColor"][1], cfg["disabledColor"][2]);
                        }
                        if (cfg.contains("activeColor")) {
                            gameUIDoubleJumpSkillConfig.activeColor = glm::vec3(cfg["activeColor"][0], cfg["activeColor"][1], cfg["activeColor"][2]);
                        }
                        if (cfg.contains("scale")) gameUIDoubleJumpSkillConfig.scale = cfg["scale"];
                        if (cfg.contains("countScale")) gameUIDoubleJumpSkillConfig.countScale = cfg["countScale"];
                        if (cfg.contains("instructionScale")) gameUIDoubleJumpSkillConfig.instructionScale = cfg["instructionScale"];
                    }
                    
                    // heartFeel
                    if (skills.contains("heartFeel")) {
                        auto& cfg = skills["heartFeel"];
                        if (cfg.contains("position")) {
                            auto& pos = cfg["position"];
                            if (pos.contains("absoluteX")) gameUIHeartFeelSkillConfig.position.absoluteX = pos["absoluteX"];
                            if (pos.contains("absoluteY")) gameUIHeartFeelSkillConfig.position.absoluteY = pos["absoluteY"];
                            if (pos.contains("useRelative")) gameUIHeartFeelSkillConfig.position.useRelative = pos["useRelative"];
                        }
                        if (cfg.contains("countOffset")) gameUIHeartFeelSkillConfig.countOffset = cfg["countOffset"];
                        if (cfg.contains("instructionOffset")) gameUIHeartFeelSkillConfig.instructionOffset = cfg["instructionOffset"];
                        if (cfg.contains("color")) {
                            gameUIHeartFeelSkillConfig.color = glm::vec3(cfg["color"][0], cfg["color"][1], cfg["color"][2]);
                        }
                        if (cfg.contains("disabledColor")) {
                            gameUIHeartFeelSkillConfig.disabledColor = glm::vec3(cfg["disabledColor"][0], cfg["disabledColor"][1], cfg["disabledColor"][2]);
                        }
                        if (cfg.contains("activeColor")) {
                            gameUIHeartFeelSkillConfig.activeColor = glm::vec3(cfg["activeColor"][0], cfg["activeColor"][1], cfg["activeColor"][2]);
                        }
                        if (cfg.contains("scale")) gameUIHeartFeelSkillConfig.scale = cfg["scale"];
                        if (cfg.contains("countScale")) gameUIHeartFeelSkillConfig.countScale = cfg["countScale"];
                        if (cfg.contains("instructionScale")) gameUIHeartFeelSkillConfig.instructionScale = cfg["instructionScale"];
                    }
                    
                    // freeCamera
                    if (skills.contains("freeCamera")) {
                        auto& cfg = skills["freeCamera"];
                        if (cfg.contains("position")) {
                            auto& pos = cfg["position"];
                            if (pos.contains("absoluteX")) gameUIFreeCameraSkillConfig.position.absoluteX = pos["absoluteX"];
                            if (pos.contains("absoluteY")) gameUIFreeCameraSkillConfig.position.absoluteY = pos["absoluteY"];
                            if (pos.contains("useRelative")) gameUIFreeCameraSkillConfig.position.useRelative = pos["useRelative"];
                        }
                        if (cfg.contains("countOffset")) gameUIFreeCameraSkillConfig.countOffset = cfg["countOffset"];
                        if (cfg.contains("instructionOffset")) gameUIFreeCameraSkillConfig.instructionOffset = cfg["instructionOffset"];
                        if (cfg.contains("color")) {
                            gameUIFreeCameraSkillConfig.color = glm::vec3(cfg["color"][0], cfg["color"][1], cfg["color"][2]);
                        }
                        if (cfg.contains("disabledColor")) {
                            gameUIFreeCameraSkillConfig.disabledColor = glm::vec3(cfg["disabledColor"][0], cfg["disabledColor"][1], cfg["disabledColor"][2]);
                        }
                        if (cfg.contains("activeColor")) {
                            gameUIFreeCameraSkillConfig.activeColor = glm::vec3(cfg["activeColor"][0], cfg["activeColor"][1], cfg["activeColor"][2]);
                        }
                        if (cfg.contains("scale")) gameUIFreeCameraSkillConfig.scale = cfg["scale"];
                        if (cfg.contains("countScale")) gameUIFreeCameraSkillConfig.countScale = cfg["countScale"];
                        if (cfg.contains("instructionScale")) gameUIFreeCameraSkillConfig.instructionScale = cfg["instructionScale"];
                        if (cfg.contains("activePosition")) {
                            auto& pos = cfg["activePosition"];
                            if (pos.contains("absoluteX")) gameUIFreeCameraSkillConfig.activePosition.absoluteX = pos["absoluteX"];
                            if (pos.contains("absoluteY")) gameUIFreeCameraSkillConfig.activePosition.absoluteY = pos["absoluteY"];
                            if (pos.contains("useRelative")) gameUIFreeCameraSkillConfig.activePosition.useRelative = pos["useRelative"];
                        }
                        if (cfg.contains("activeScale")) gameUIFreeCameraSkillConfig.activeScale = cfg["activeScale"];
                    }
                    
                    // burstJump
                    if (skills.contains("burstJump")) {
                        auto& cfg = skills["burstJump"];
                        if (cfg.contains("position")) {
                            auto& pos = cfg["position"];
                            if (pos.contains("absoluteX")) gameUIBurstJumpSkillConfig.position.absoluteX = pos["absoluteX"];
                            if (pos.contains("absoluteY")) gameUIBurstJumpSkillConfig.position.absoluteY = pos["absoluteY"];
                            if (pos.contains("useRelative")) gameUIBurstJumpSkillConfig.position.useRelative = pos["useRelative"];
                        }
                        if (cfg.contains("countOffset")) gameUIBurstJumpSkillConfig.countOffset = cfg["countOffset"];
                        if (cfg.contains("instructionOffset")) gameUIBurstJumpSkillConfig.instructionOffset = cfg["instructionOffset"];
                        if (cfg.contains("color")) {
                            gameUIBurstJumpSkillConfig.color = glm::vec3(cfg["color"][0], cfg["color"][1], cfg["color"][2]);
                        }
                        if (cfg.contains("disabledColor")) {
                            gameUIBurstJumpSkillConfig.disabledColor = glm::vec3(cfg["disabledColor"][0], cfg["disabledColor"][1], cfg["disabledColor"][2]);
                        }
                        if (cfg.contains("activeColor")) {
                            gameUIBurstJumpSkillConfig.activeColor = glm::vec3(cfg["activeColor"][0], cfg["activeColor"][1], cfg["activeColor"][2]);
                        }
                        if (cfg.contains("scale")) gameUIBurstJumpSkillConfig.scale = cfg["scale"];
                        if (cfg.contains("countScale")) gameUIBurstJumpSkillConfig.countScale = cfg["countScale"];
                        if (cfg.contains("instructionScale")) gameUIBurstJumpSkillConfig.instructionScale = cfg["instructionScale"];
                    }
                }
            }
            
            configLoaded = true;
            
            // ファイルの更新時刻を記録
            lastFileModificationTime = getFileModificationTime(configFilePath);
            
            printf("UI Config: Loaded successfully from %s\n", configFilePath.c_str());
            return true;
            
        } catch (const std::exception& e) {
            printf("UI Config: Error loading config: %s\n", e.what());
            configLoaded = false;
            return false;
        }
    }
    
    void UIConfigManager::reloadConfig() {
        if (!configFilePath.empty()) {
            loadConfig(configFilePath);
        }
    }
    
    time_t UIConfigManager::getFileModificationTime(const std::string& filepath) const {
#ifdef _WIN32
        struct _stat fileInfo;
        if (_stat(filepath.c_str(), &fileInfo) == 0) {
            return fileInfo.st_mtime;
        }
        // 代替パスも試す
        std::string altPath = filepath;
        if (altPath.find("../") == 0) {
            altPath = altPath.substr(3);
        } else if (altPath.find("assets/") == 0) {
            altPath = "../" + altPath;
        }
        if (_stat(altPath.c_str(), &fileInfo) == 0) {
            return fileInfo.st_mtime;
        }
#else
        struct stat fileInfo;
        if (stat(filepath.c_str(), &fileInfo) == 0) {
            return fileInfo.st_mtime;
        }
        // 代替パスも試す
        std::string altPath = filepath;
        if (altPath.find("../") == 0) {
            altPath = altPath.substr(3);
        } else if (altPath.find("assets/") == 0) {
            altPath = "../" + altPath;
        }
        if (stat(altPath.c_str(), &fileInfo) == 0) {
            return fileInfo.st_mtime;
        }
#endif
        return 0;
    }
    
    bool UIConfigManager::checkAndReloadConfig() {
        // ファイルパスが設定されていない場合はスキップ
        if (configFilePath.empty()) {
            return false;
        }
        
        // ファイルの更新時刻を取得
        time_t currentModTime = getFileModificationTime(configFilePath);
        
        // 更新時刻が変わった場合はリロード
        if (currentModTime > 0 && currentModTime != lastFileModificationTime && lastFileModificationTime > 0) {
            printf("UI Config: File changed! Reloading...\n");
            lastFileModificationTime = currentModTime;
            reloadConfig();
            return true;
        }
        
        // 初回の場合は更新時刻を記録
        if (lastFileModificationTime == 0 && currentModTime > 0) {
            lastFileModificationTime = currentModTime;
        }
        
        return false;
    }
    
    glm::vec2 UIConfigManager::calculatePosition(const UIPosition& pos, int windowWidth, int windowHeight) const {
        if (pos.useRelative) {
            // 相対位置（画面中央基準）
            return glm::vec2(windowWidth / 2.0f + pos.offsetX, windowHeight / 2.0f + pos.offsetY);
        } else {
            // 絶対位置（画面左上基準）
            float y = pos.absoluteY;
            // offsetYが設定されている場合は画面下からの相対位置として扱う
            if (pos.offsetY != 0.0f) {
                y = windowHeight + pos.offsetY;
            }
            return glm::vec2(pos.absoluteX, y);
        }
    }
    
    // JSONパスから値を取得（内部用）
    nlohmann::json UIConfigManager::getJsonValue(const std::string& jsonPath) const {
        if (!configLoaded || cachedJsonData.is_null()) {
            return nlohmann::json();
        }
        
        // パスを分割（例: "gameUI.timeDisplay" -> ["gameUI", "timeDisplay"]）
        std::vector<std::string> parts;
        std::string current;
        for (char c : jsonPath) {
            if (c == '.') {
                if (!current.empty()) {
                    parts.push_back(current);
                    current.clear();
                }
            } else {
                current += c;
            }
        }
        if (!current.empty()) {
            parts.push_back(current);
        }
        
        // JSONを辿る
        nlohmann::json result = cachedJsonData;
        for (const auto& part : parts) {
            if (result.is_object() && result.contains(part)) {
                result = result[part];
            } else {
                return nlohmann::json();  // パスが見つからない
            }
        }
        
        return result;
    }
    
    // JSONからUIPositionを解析
    UIPosition UIConfigManager::parseUIPosition(const nlohmann::json& json) const {
        UIPosition pos;
        if (json.is_null() || !json.is_object()) {
            return pos;
        }
        
        if (json.contains("position") && json["position"].is_object()) {
            auto& posJson = json["position"];
            if (posJson.contains("useRelative")) pos.useRelative = posJson["useRelative"];
            if (posJson.contains("offsetX")) pos.offsetX = posJson["offsetX"];
            if (posJson.contains("offsetY")) pos.offsetY = posJson["offsetY"];
            if (posJson.contains("absoluteX")) pos.absoluteX = posJson["absoluteX"];
            if (posJson.contains("absoluteY")) pos.absoluteY = posJson["absoluteY"];
        } else if (json.contains("useRelative") || json.contains("offsetX") || json.contains("absoluteX")) {
            // positionオブジェクトがなく、直接位置情報が含まれている場合
            if (json.contains("useRelative")) pos.useRelative = json["useRelative"];
            if (json.contains("offsetX")) pos.offsetX = json["offsetX"];
            if (json.contains("offsetY")) pos.offsetY = json["offsetY"];
            if (json.contains("absoluteX")) pos.absoluteX = json["absoluteX"];
            if (json.contains("absoluteY")) pos.absoluteY = json["absoluteY"];
        }
        
        return pos;
    }
    
    // JSONからUITextConfigを解析
    UITextConfig UIConfigManager::parseUITextConfig(const nlohmann::json& json) const {
        UITextConfig config;
        if (json.is_null() || !json.is_object()) {
            return config;
        }
        
        config.position = parseUIPosition(json);
        
        if (json.contains("color") && json["color"].is_array() && json["color"].size() >= 3) {
            config.color = glm::vec3(json["color"][0], json["color"][1], json["color"][2]);
        }
        
        if (json.contains("completedColor") && json["completedColor"].is_array() && json["completedColor"].size() >= 3) {
            config.completedColor = glm::vec3(json["completedColor"][0], json["completedColor"][1], json["completedColor"][2]);
        }
        
        if (json.contains("scale")) {
            config.scale = json["scale"];
        }
        
        return config;
    }
    
    // JSONからUISelectableConfigを解析
    UISelectableConfig UIConfigManager::parseUISelectableConfig(const nlohmann::json& json) const {
        UISelectableConfig config;
        if (json.is_null() || !json.is_object()) {
            return config;
        }
        
        config.position = parseUIPosition(json);
        
        if (json.contains("selectedColor") && json["selectedColor"].is_array() && json["selectedColor"].size() >= 3) {
            config.selectedColor = glm::vec3(json["selectedColor"][0], json["selectedColor"][1], json["selectedColor"][2]);
        }
        
        if (json.contains("unselectedColor") && json["unselectedColor"].is_array() && json["unselectedColor"].size() >= 3) {
            config.unselectedColor = glm::vec3(json["unselectedColor"][0], json["unselectedColor"][1], json["unselectedColor"][2]);
        }
        
        if (json.contains("scale")) {
            config.scale = json["scale"];
        }
        
        if (json.contains("spacing")) {
            config.spacing = json["spacing"];
        }
        
        return config;
    }
    
    // JSONからUITimeDisplayConfigを解析
    UITimeDisplayConfig UIConfigManager::parseUITimeDisplayConfig(const nlohmann::json& json) const {
        UITimeDisplayConfig config;
        if (json.is_null() || !json.is_object()) {
            return config;
        }
        
        config.position = parseUIPosition(json);
        
        if (json.contains("normalColor") && json["normalColor"].is_array() && json["normalColor"].size() >= 3) {
            config.normalColor = glm::vec3(json["normalColor"][0], json["normalColor"][1], json["normalColor"][2]);
        }
        
        if (json.contains("warningColor") && json["warningColor"].is_array() && json["warningColor"].size() >= 3) {
            config.warningColor = glm::vec3(json["warningColor"][0], json["warningColor"][1], json["warningColor"][2]);
        }
        
        if (json.contains("scale")) {
            config.scale = json["scale"];
        }
        
        return config;
    }
    
    // JSONからUISkillConfigを解析
    UISkillConfig UIConfigManager::parseUISkillConfig(const nlohmann::json& json) const {
        UISkillConfig config;
        if (json.is_null() || !json.is_object()) {
            return config;
        }
        
        config.position = parseUIPosition(json);
        
        if (json.contains("countOffset")) {
            config.countOffset = json["countOffset"];
        }
        
        if (json.contains("instructionOffset")) {
            config.instructionOffset = json["instructionOffset"];
        }
        
        if (json.contains("color") && json["color"].is_array() && json["color"].size() >= 3) {
            config.color = glm::vec3(json["color"][0], json["color"][1], json["color"][2]);
        }
        
        if (json.contains("disabledColor") && json["disabledColor"].is_array() && json["disabledColor"].size() >= 3) {
            config.disabledColor = glm::vec3(json["disabledColor"][0], json["disabledColor"][1], json["disabledColor"][2]);
        }
        
        if (json.contains("activeColor") && json["activeColor"].is_array() && json["activeColor"].size() >= 3) {
            config.activeColor = glm::vec3(json["activeColor"][0], json["activeColor"][1], json["activeColor"][2]);
        }
        
        if (json.contains("scale")) {
            config.scale = json["scale"];
        }
        
        if (json.contains("countScale")) {
            config.countScale = json["countScale"];
        }
        
        if (json.contains("instructionScale")) {
            config.instructionScale = json["instructionScale"];
        }
        
        if (json.contains("activePosition") && json["activePosition"].is_object()) {
            config.activePosition = parseUIPosition(json["activePosition"]);
        }
        
        if (json.contains("activeScale")) {
            config.activeScale = json["activeScale"];
        }
        
        return config;
    }
    
}

