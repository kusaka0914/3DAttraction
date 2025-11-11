#pragma once
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <string>
#include "../core/constants/game_constants.h"
#include "bitmap_font.h"

namespace gfx {
class GameStateUIRenderer {
public:
    GameStateUIRenderer();
    ~GameStateUIRenderer();
    void renderTutorialStageUI(int width, int height, const std::string& message, int currentStep, bool stepCompleted);
    void renderStageClearBackground(int width, int height, float clearTime, int earnedStars);
    void renderUnlockConfirmBackground(int width, int height, int targetStage, int requiredStars, int currentStars);
    void renderStarInsufficientBackground(int width, int height, int targetStage, int requiredStars, int currentStars);
    void renderWarpTutorialBackground(int width, int height, int targetStage);
    void renderGameOverBackground(int width, int height);
    void renderReadyScreen(int width, int height, int speedLevel, bool isFirstPersonMode);
    void renderCountdown(int width, int height, int count);
    void renderStage0Tutorial(int width, int height);
    void renderEasyModeExplanationUI(int width, int height);
    void renderModeSelectionUI(int width, int height, bool isEasyMode);
    void renderStageSelectionAssist(int width, int height, int targetStage, bool isVisible, bool isUnlocked);
    void renderEndingMessage(int width, int height, float timer);
    void renderStaffRoll(int width, int height, float timer);
    void begin2DMode();
    void end2DMode();
private:
    void renderText(const std::string& text, const glm::vec2& position, const glm::vec3& color, float scale = 1.0f);
    void renderBitmapChar(char c, const glm::vec2& position, const glm::vec3& color, float scale);
    void renderStar(const glm::vec2& position, const glm::vec3& color, float scale, int width, int height);
    
    BitmapFont font; // ビットマップフォントのインスタンス
};
} // namespace gfx
