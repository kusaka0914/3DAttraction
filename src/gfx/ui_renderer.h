#pragma once
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <string>
#include "../app/game_constants.h"
#include "bitmap_font.h"

namespace gfx {
class UIRenderer {
public:
    UIRenderer();
    ~UIRenderer();
    void renderTimeUI(float remainingTime, float timeLimit, int earnedStars, int existingStars, int lives);
    void renderLivesOnly(int lives);
    void renderTimeUIOnly(float remainingTime, float timeLimit, int earnedStars, int existingStars);
    void renderLivesWithExplanation(int lives);
    void renderLivesAndTimeUI(int lives, float remainingTime, float timeLimit, int earnedStars, int existingStars);
    void renderLivesTimeAndStarsUI(int lives, float remainingTime, float timeLimit, int earnedStars, int existingStars);
    void renderTimeStopUI(bool hasSkill, bool isTimeStopped, float timeStopTimer, int remainingUses, int maxUses);
    void renderDoubleJumpUI(bool hasSkill, bool isEasyMode, int remainingUses, int maxUses);
    void renderHeartFeelUI(bool hasSkill, int remainingUses, int maxUses, int currentLives);
    void renderFreeCameraUI(bool hasSkill, bool isActive, float timer, int remainingUses, int maxUses);
    void renderBurstJumpUI(bool hasSkill, bool isActive, int remainingUses, int maxUses);
    void renderStar(const glm::vec2& position, const glm::vec3& color, float scale);
    void renderHeart(const glm::vec2& position, const glm::vec3& color, float scale);
    void renderText(const std::string& text, const glm::vec2& position, const glm::vec3& color, float scale = 1.0f);
    void begin2DMode();
    void end2DMode();
private:
    void renderBitmapChar(char c, const glm::vec2& position, const glm::vec3& color, float scale);
    
    BitmapFont font; // ビットマップフォントのインスタンス
};
} // namespace gfx
