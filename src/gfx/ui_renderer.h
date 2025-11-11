#pragma once
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <string>
#include "../core/constants/game_constants.h"
#include "bitmap_font.h"

namespace gfx {

// UI状態を管理する構造体
struct GameUIState {
    bool showTime = true;
    bool showGoal = true;
    bool showStars = true;
    bool showLives = true;
    
    float remainingTime = 0.0f;
    float timeLimit = 0.0f;
    int existingStars = 0;
    int lives = 6;
};

// スキルUI設定を管理する構造体
struct SkillUIConfig {
    std::string skillName;
    std::string instructionText;
    float skillX;
    float countOffset;
    float instructionOffset;
    bool hasActiveState;
    std::string activeText;
    glm::vec2 activePosition;
    
    SkillUIConfig(const std::string& name, const std::string& instruction, 
                   float x, float countOff, float instructionOff, 
                   bool active = false, const std::string& activeTxt = "", 
                   const glm::vec2& activePos = glm::vec2(0, 0))
        : skillName(name), instructionText(instruction), skillX(x), 
          countOffset(countOff), instructionOffset(instructionOff),
          hasActiveState(active), activeText(activeTxt), activePosition(activePos) {}
};

class UIRenderer {
public:
    UIRenderer();
    ~UIRenderer();
    
    // ウィンドウサイズを設定（スケーリング用）
    void setWindowSize(int width, int height);
    
    // 統合されたUI描画関数
    void renderGameUI(const GameUIState& state);
    
    // 個別UI描画関数（後方互換性のため残す）
    void renderTimeUI(float remainingTime, float timeLimit, int earnedStars, int existingStars, int lives);
    void renderLivesWithExplanation(int lives);
    void renderLivesAndTimeUI(int lives, float remainingTime, float timeLimit, int earnedStars, int existingStars);
    void renderLivesTimeAndStarsUI(int lives, float remainingTime, float timeLimit, int earnedStars, int existingStars);
    
    // 統合されたスキルUI描画関数
    void renderSkillUI(const SkillUIConfig& config, bool hasSkill, bool isActive, 
                       int remainingUses, int maxUses, bool isEasyMode = false);
    
    // 個別スキルUI描画関数（後方互換性のため）
    void renderTimeStopUI(bool hasSkill, bool isTimeStopped, float timeStopTimer, int remainingUses, int maxUses);
    void renderDoubleJumpUI(bool hasSkill, bool isEasyMode, int remainingUses, int maxUses);
    void renderHeartFeelUI(bool hasSkill, int remainingUses, int maxUses, int currentLives);
    void renderFreeCameraUI(bool hasSkill, bool isActive, float timer, int remainingUses, int maxUses);
    void renderBurstJumpUI(bool hasSkill, bool isActive, int remainingUses, int maxUses);
    
    // 基本描画関数
    void renderStar(const glm::vec2& position, const glm::vec3& color, float scale);
    void renderHeart(const glm::vec2& position, const glm::vec3& color, float scale);
    void renderText(const std::string& text, const glm::vec2& position, const glm::vec3& color, float scale = 1.0f);
    
    // 2D描画モード管理
    void begin2DMode();
    void end2DMode();

private:
    // 個別UIコンポーネント描画関数
    void renderTimeDisplay(float remainingTime, float timeLimit);
    void renderGoalDisplay(float timeLimit);
    void renderStarsDisplay(int existingStars);
    void renderLivesDisplay(int lives);
    
    // 説明テキストの共通化関数
    void renderExplanationText(const std::string& type, const glm::vec2& position);
    
    void renderBitmapChar(char c, const glm::vec2& position, const glm::vec3& color, float scale);
    
    BitmapFont font;
    
    // ウィンドウサイズ（スケーリング用）
    int windowWidth = 1280;
    int windowHeight = 720;
};

} // namespace gfx
