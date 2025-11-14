#include "ui_renderer.h"
#include "bitmap_font.h"
#include "../core/utils/ui_config_manager.h"
#include <iostream>

namespace gfx {

UIRenderer::UIRenderer() {
    font.initialize(); // フォントを初期化
}

UIRenderer::~UIRenderer() {
}

void UIRenderer::setWindowSize(int width, int height) {
    windowWidth = width;
    windowHeight = height;
}

void UIRenderer::begin2DMode() {
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, windowWidth, windowHeight, 0, -1, 1);
    
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    
    glDisable(GL_DEPTH_TEST);
}

void UIRenderer::end2DMode() {
    glEnable(GL_DEPTH_TEST);
    
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

void UIRenderer::renderText(const std::string& text, const glm::vec2& position, const glm::vec3& color, float scale) {
    begin2DMode();
    
    float scaleX = static_cast<float>(windowWidth) / 1280.0f;
    float scaleY = static_cast<float>(windowHeight) / 720.0f;
    glm::vec2 scaledPosition = glm::vec2(position.x * scaleX, position.y * scaleY);
    float scaledScale = scale * std::min(scaleX, scaleY);  // アスペクト比を維持
    
    glColor3f(color.r, color.g, color.b);
    
    float currentX = scaledPosition.x;
    float charWidth = GameConstants::RenderConstants::CHAR_WIDTH * scaledScale;
    float charHeight = GameConstants::RenderConstants::CHAR_HEIGHT * scaledScale;
    float spaceWidth = GameConstants::RenderConstants::SPACE_WIDTH * scaledScale;
    
    for (size_t i = 0; i < text.length(); i++) {
        char c = text[i];
        
        if (c == ' ') {
            currentX += spaceWidth;
            continue;
        }
        
        renderBitmapChar(c, glm::vec2(currentX, scaledPosition.y), color, scaledScale);
        currentX += charWidth + GameConstants::RenderConstants::CHAR_SPACING * scaledScale;  // 文字間隔を増加
    }
    
    end2DMode();
}

void UIRenderer::renderBitmapChar(char c, const glm::vec2& position, const glm::vec3& color, float scale) {
    if (!font.hasCharacter(c)) {
        return;
    }
    
    const std::vector<bool>& charData = font.getCharacter(c);
    
    glColor3f(color.r, color.g, color.b);
    
    for (int y = 0; y < 12; y++) {
        for (int x = 0; x < 8; x++) {
            size_t index = y * 8 + x;
            if (index < charData.size() && charData[index]) {
                float pixelX = position.x + x * scale;
                float pixelY = position.y + y * scale;
                
                glBegin(GL_QUADS);
                glVertex2f(pixelX, pixelY);
                glVertex2f(pixelX + scale, pixelY);
                glVertex2f(pixelX + scale, pixelY + scale);
                glVertex2f(pixelX, pixelY + scale);
                glEnd();
            }
        }
    }
}

void UIRenderer::renderGameUI(const GameUIState& state) {
    begin2DMode();
    
    if (state.showTime) {
        renderTimeDisplay(state.remainingTime, state.timeLimit);
    }
    
    if (state.showGoal) {
        renderGoalDisplay(state.timeLimit);
    }
    
    if (state.showStars) {
        renderStarsDisplay(state.existingStars, state.currentStage, state.selectedSecretStarType, state.secretStarCleared);
    }
    
    if (state.showLives) {
        renderLivesDisplay(state.lives);
    }
    
    end2DMode();
}

void UIRenderer::renderTimeUI(float remainingTime, float timeLimit, int earnedStars, int existingStars, int lives,
                               int currentStage, int selectedSecretStarType, const std::map<int, std::set<int>>& secretStarCleared) {
    GameUIState state;
    state.showTime = true;
    state.showGoal = (selectedSecretStarType < 0);
    state.showStars = true;
    state.showLives = true;
    state.remainingTime = remainingTime;
    state.timeLimit = timeLimit;
    state.existingStars = existingStars;
    state.lives = lives;
    state.currentStage = currentStage;
    state.selectedSecretStarType = selectedSecretStarType;
    state.secretStarCleared = secretStarCleared;
    
    renderGameUI(state);
}

void UIRenderer::renderTimeAttackUI(float currentTime, float bestTime, int earnedStars, int existingStars, int lives, float timeScale, bool isReplayMode) {
    begin2DMode();
    
    renderTimeAttackDisplay(currentTime, bestTime);
    
    
    auto& uiConfig = UIConfig::UIConfigManager::getInstance();
    auto speedConfig = uiConfig.getSpeedDisplayConfig();
    auto pressTConfig = uiConfig.getPressTConfig();
    auto timeAttackSpeedPos = uiConfig.getGameUITimeAttackSpeedDisplayPosition();
    auto timeAttackPressTPos = uiConfig.getGameUITimeAttackPressTPosition();
    
    glm::vec2 speedPos = uiConfig.calculatePosition(timeAttackSpeedPos, windowWidth, windowHeight);
    std::string speedText = std::to_string((int)timeScale) + "x";
    glm::vec3 speedColor = (timeScale > 1.0f) ? glm::vec3(1.0f, 0.8f, 0.2f) : speedConfig.color;
    renderText(speedText, speedPos, speedColor, speedConfig.scale);
    
    if (!isReplayMode) {
        glm::vec2 pressTPos = uiConfig.calculatePosition(timeAttackPressTPos, windowWidth, windowHeight);
        glm::vec3 pressTColor = (timeScale > 1.0f) ? glm::vec3(1.0f, 0.8f, 0.2f) : pressTConfig.color;
        renderText("PRESS T", pressTPos, pressTColor, pressTConfig.scale);
    }
    
    if (lives > 0) {
        renderLivesDisplay(lives);
    }
    
    end2DMode();
}

void UIRenderer::renderTimeDisplay(float remainingTime, float timeLimit) {
    auto& uiConfig = UIConfig::UIConfigManager::getInstance();
    auto timeConfig = uiConfig.getGameUITimeDisplayConfig();
    glm::vec2 timePos = uiConfig.calculatePosition(timeConfig.position, windowWidth, windowHeight);
    
    std::string timeText = std::to_string(static_cast<int>(remainingTime)) + "s";
    glm::vec3 timeColor = (remainingTime <= 5.0f) ? timeConfig.warningColor : timeConfig.normalColor;
    
    renderText(timeText, timePos, timeColor, timeConfig.scale);
}

void UIRenderer::renderTimeAttackDisplay(float currentTime, float bestTime) {
    auto& uiConfig = UIConfig::UIConfigManager::getInstance();
    auto timeAttackConfig = uiConfig.getGameUITimeAttackDisplayConfig();
    auto bestTimeConfig = uiConfig.getGameUIBestTimeConfig();
    
    int minutes = static_cast<int>(currentTime) / 60;
    int seconds = static_cast<int>(currentTime) % 60;
    int milliseconds = static_cast<int>((currentTime - static_cast<int>(currentTime)) * 100);
    
    std::string timeText = (minutes > 0 ? std::to_string(minutes) + ":" : "") + 
                          (seconds < 10 ? "0" : "") + std::to_string(seconds) + "." +
                          (milliseconds < 10 ? "0" : "") + std::to_string(milliseconds);
    
    glm::vec2 timeAttackPos = uiConfig.calculatePosition(timeAttackConfig.position, windowWidth, windowHeight);
    renderText(timeText, timeAttackPos, timeAttackConfig.color, timeAttackConfig.scale);
    
}

void UIRenderer::renderGoalDisplay(float timeLimit) {
    auto& uiConfig = UIConfig::UIConfigManager::getInstance();
    auto goalConfig = uiConfig.getGameUIGoalDisplayConfig();
    auto goal5sConfig = uiConfig.getGameUIGoalTime5sConfig();
    auto goal10sConfig = uiConfig.getGameUIGoalTime10sConfig();
    auto goal20sConfig = uiConfig.getGameUIGoalTime20sConfig();
    
    glm::vec2 goalPos = uiConfig.calculatePosition(goalConfig.position, windowWidth, windowHeight);
    renderText("GOAL", goalPos, goalConfig.color, goalConfig.scale);
    
    if (timeLimit <= 20) {
        glm::vec2 goal5sPos = uiConfig.calculatePosition(goal5sConfig.position, windowWidth, windowHeight);
        glm::vec2 goal10sPos = uiConfig.calculatePosition(goal10sConfig.position, windowWidth, windowHeight);
        renderText("5s", goal5sPos, goal5sConfig.color, goal5sConfig.scale);
        renderText("10s", goal10sPos, goal10sConfig.color, goal10sConfig.scale);
    } else {
        glm::vec2 goal5sPos = uiConfig.calculatePosition(goal5sConfig.position, windowWidth, windowHeight);
        glm::vec2 goal20sPos = uiConfig.calculatePosition(goal20sConfig.position, windowWidth, windowHeight);
        renderText("10s", goal5sPos, goal5sConfig.color, goal5sConfig.scale);
        renderText("20s", goal20sPos, goal20sConfig.color, goal20sConfig.scale);
    }
}

void UIRenderer::renderStarsDisplay(int existingStars, int currentStage, int selectedSecretStarType, const std::map<int, std::set<int>>& secretStarCleared) {
    auto& uiConfig = UIConfig::UIConfigManager::getInstance();
    auto starsConfig = uiConfig.getGameUIStarsConfig();
    glm::vec2 basePos = uiConfig.calculatePosition(starsConfig.position, windowWidth, windowHeight);
    
    if (selectedSecretStarType >= 0 && currentStage >= 0) {
        std::vector<int> secretStarTypes = {0, 1, 2}; // 0=MAX_SPEED_STAR, 1=SHADOW_STAR, 2=IMMERSIVE_STAR
        
        std::vector<glm::vec3> secretStarColors = {
            glm::vec3(0.2f, 0.8f, 1.0f),  // MAX_SPEED_STAR: 水色
            glm::vec3(0.1f, 0.1f, 0.1f),  // SHADOW_STAR: 黒
            glm::vec3(1.0f, 0.4f, 0.8f)   // IMMERSIVE_STAR: ピンク
        };
        
        glm::vec3 inactiveColor = glm::vec3(0.5f, 0.5f, 0.5f); // 灰色（未獲得）
        
        std::set<int> clearedTypes;
        if (secretStarCleared.count(currentStage) > 0) {
            clearedTypes = secretStarCleared.at(currentStage);
        }
        
        for (int i = 0; i < 3; i++) {
            glm::vec2 starPos = glm::vec2(basePos.x + i * starsConfig.spacing, basePos.y);
            bool isCleared = (clearedTypes.count(secretStarTypes[i]) > 0);
            glm::vec3 starColor = isCleared ? secretStarColors[secretStarTypes[i]] : inactiveColor;
            renderStar(starPos, starColor, starsConfig.scale);
        }
    } else {
        for (int i = 0; i < 3; i++) {
            glm::vec2 starPos = glm::vec2(basePos.x + i * starsConfig.spacing, basePos.y);
            glm::vec3 starColor = (i < existingStars) ? starsConfig.selectedColor : starsConfig.unselectedColor;
            renderStar(starPos, starColor, starsConfig.scale);
        }
    }
}

void UIRenderer::renderLivesDisplay(int lives) {
    auto& uiConfig = UIConfig::UIConfigManager::getInstance();
    auto heartsConfig = uiConfig.getGameUIHeartsConfig();
    glm::vec2 basePos = uiConfig.calculatePosition(heartsConfig.position, windowWidth, windowHeight);
    
    for (int i = 0; i < 6; i++) {
        glm::vec2 heartPos = glm::vec2(basePos.x + i * heartsConfig.spacing, basePos.y);
        glm::vec3 heartColor = (i < lives) ? heartsConfig.selectedColor : heartsConfig.unselectedColor;
        renderHeart(heartPos, heartColor, heartsConfig.scale);
    }
}

void UIRenderer::renderExplanationText(const std::string& type, const glm::vec2& position) {
    if (type == "lives") {
        std::string explanation1 = "THESE ARE YOUR LIVES !";
        std::string explanation2 = "THEY DECREASE AS YOU FALL !";
        std::string explanation3 = "IF THEY'RE ALL GONE, IT'S GAME OVER !";
        std::string explanation4 = "SO BE CAREFUL !";

        int offsetY = position.y - 100;
        int interval = 40;
        
        renderText(explanation1, glm::vec2(position.x - 460, offsetY), GameConstants::Colors::UI_TEXT_COLOR, GameConstants::Colors::UILayout::EXPLANATION_SCALE);
        renderText(explanation2, glm::vec2(position.x - 480, offsetY + interval), GameConstants::Colors::UI_TEXT_COLOR, GameConstants::Colors::UILayout::EXPLANATION_SCALE);
        renderText(explanation3, glm::vec2(position.x - 520, offsetY + interval * 2), GameConstants::Colors::UI_TEXT_COLOR, GameConstants::Colors::UILayout::EXPLANATION_SCALE);
        renderText(explanation4, glm::vec2(position.x - 420, offsetY + interval * 3), GameConstants::Colors::UI_TEXT_COLOR, GameConstants::Colors::UILayout::EXPLANATION_SCALE);
    }
    else if (type == "time") {
        std::string timeLimitText = "THIS IS THE TIME LIMIT!";
        std::string timeLimitText2 = "IF IT REACHES 0, THE GAME IS OVER!";
        std::string timeLimitText3 = "AIM FOR THE GOAL WITHIN THE TIME LIMIT!";

        int offsetY = position.y;
        int interval = 40;
        
        renderText(timeLimitText, glm::vec2(position.x, offsetY), GameConstants::Colors::UI_TEXT_COLOR, GameConstants::Colors::UILayout::EXPLANATION_SCALE);
        renderText(timeLimitText2, glm::vec2(position.x, offsetY + interval), GameConstants::Colors::UI_TEXT_COLOR, GameConstants::Colors::UILayout::EXPLANATION_SCALE);
        renderText(timeLimitText3, glm::vec2(position.x, offsetY + interval * 2), GameConstants::Colors::UI_TEXT_COLOR, GameConstants::Colors::UILayout::EXPLANATION_SCALE);
    }
    else if (type == "stars") {
        std::string starsText = "THESE ARE STARS !";
        std::string starsText2 = "YOU CAN GET THEM BY CLEARING A STAGE !";
        std::string starsText3 = "THE MORE TIME YOU HAVE LEFT, THE MORE STARS YOU'LL GET !";
        std::string starsText4 = "CLEAR THE STAGE QUICKLY TO COLLECT AS MANY STARS AS POSSIBLE !";

        int offsetY = position.y;
        int interval = 40;
        
        renderText(starsText, glm::vec2(position.x, offsetY), GameConstants::Colors::UI_TEXT_COLOR, GameConstants::Colors::UILayout::EXPLANATION_SCALE);
        renderText(starsText2, glm::vec2(position.x, offsetY + interval), GameConstants::Colors::UI_TEXT_COLOR, GameConstants::Colors::UILayout::EXPLANATION_SCALE);
        renderText(starsText3, glm::vec2(position.x, offsetY + interval * 2), GameConstants::Colors::UI_TEXT_COLOR, GameConstants::Colors::UILayout::EXPLANATION_SCALE);
        renderText(starsText4, glm::vec2(position.x, offsetY + interval * 3), GameConstants::Colors::UI_TEXT_COLOR, GameConstants::Colors::UILayout::EXPLANATION_SCALE);
    }
}

void UIRenderer::renderLivesWithExplanation(int lives) {
    GameUIState state;
    state.showLives = true;
    state.showTime = false;
    state.showGoal = false;
    state.showStars = false;
    state.lives = lives;
    
    renderGameUI(state);
    
    renderExplanationText("lives", glm::vec2(640.0f, 200.0f));
}

void UIRenderer::renderLivesAndTimeUI(int lives, float remainingTime, float timeLimit, int earnedStars, int existingStars) {
    GameUIState state;
    state.showTime = true;
    state.showLives = true;
    state.remainingTime = remainingTime;
    state.timeLimit = timeLimit;
    state.lives = lives;
    state.showStars = false;
    state.showGoal = false;
    
    renderGameUI(state);
    
    renderExplanationText("time", glm::vec2(800, 120));
}

void UIRenderer::renderLivesTimeAndStarsUI(int lives, float remainingTime, float timeLimit, int earnedStars, int existingStars,
                                            int currentStage, int selectedSecretStarType, const std::map<int, std::set<int>>& secretStarCleared) {
    GameUIState state;
    state.showTime = true;
    state.showGoal = (selectedSecretStarType < 0);
    state.showStars = true;
    state.showLives = true;
    state.remainingTime = remainingTime;
    state.timeLimit = timeLimit;
    state.existingStars = existingStars;
    state.lives = lives;
    state.currentStage = currentStage;
    state.selectedSecretStarType = selectedSecretStarType;
    state.secretStarCleared = secretStarCleared;
    
    renderGameUI(state);
    
    renderExplanationText("stars", glm::vec2(590, 100));
}

void UIRenderer::renderSkillUI(const SkillUIConfig& config, bool hasSkill, bool isActive, 
                               int remainingUses, int maxUses, bool isEasyMode) {
    if (!hasSkill) return;
    
    begin2DMode();
    
    std::string skillText = config.skillName;
    glm::vec3 skillColor = GameConstants::Colors::UI_TEXT_COLOR;
    glm::vec3 usesColor = GameConstants::Colors::UI_TEXT_COLOR;
    
    if (remainingUses <= 0) {
        usesColor = GameConstants::Colors::UI_DISABLED_COLOR;
        skillColor = GameConstants::Colors::UI_DISABLED_COLOR;
    }
    
    if (isActive) {
        usesColor = GameConstants::Colors::UI_ACTIVE_COLOR;
        skillColor = GameConstants::Colors::UI_ACTIVE_COLOR;
    }
    
    renderText(skillText, glm::vec2(config.skillX, GameConstants::Colors::UILayout::SKILL_START_Y), skillColor, GameConstants::Colors::UILayout::SKILL_SCALE);
    
    renderText(std::to_string(remainingUses), glm::vec2(config.skillX + config.countOffset, GameConstants::Colors::UILayout::SKILL_COUNT_Y), usesColor, GameConstants::Colors::UILayout::SKILL_COUNT_SCALE);
    
    renderText(config.instructionText, glm::vec2(config.skillX + config.instructionOffset, GameConstants::Colors::UILayout::SKILL_INSTRUCTION_Y), usesColor, GameConstants::Colors::UILayout::SKILL_INSTRUCTION_SCALE);
    
    if (config.hasActiveState && isActive) {
        renderText(config.activeText, config.activePosition, GameConstants::Colors::UI_ACTIVE_COLOR, GameConstants::Colors::UILayout::SKILL_ACTIVE_SCALE);
    }
    
    end2DMode();
}

void UIRenderer::renderTimeStopUI(bool hasSkill, bool isTimeStopped, float timeStopTimer, int remainingUses, int maxUses) {
    if (!hasSkill) return;
    
    SkillUIConfig config("TIME STOP", "PRESS Q", 
                        GameConstants::Colors::UILayout::TIME_STOP_SKILL_X,
                        GameConstants::Colors::UILayout::TIME_STOP_COUNT_OFFSET,
                        GameConstants::Colors::UILayout::TIME_STOP_INSTRUCTION_OFFSET,
                        true, "TIME STOPPED", glm::vec2(0, 0)); // アクティブ状態あり
    
    renderSkillUI(config, hasSkill, isTimeStopped, remainingUses, maxUses);
}

void UIRenderer::renderDoubleJumpUI(bool hasSkill, bool isEasyMode, int remainingUses, int maxUses) {
    if (isEasyMode || !hasSkill) return;
    
    SkillUIConfig config("DOUBLE JUMP", "PRESS SPACE IN AIR", 
                        GameConstants::Colors::UILayout::DOUBLE_JUMP_SKILL_X,
                        GameConstants::Colors::UILayout::DOUBLE_JUMP_COUNT_OFFSET,
                        GameConstants::Colors::UILayout::DOUBLE_JUMP_INSTRUCTION_OFFSET);
    
    renderSkillUI(config, hasSkill, false, remainingUses, maxUses);
}

void UIRenderer::renderHeartFeelUI(bool hasSkill, int remainingUses, int maxUses, int currentLives) {
    if (!hasSkill) return;
    
    SkillUIConfig config("HEART FEEL", "PRESS H", 
                        GameConstants::Colors::UILayout::HEART_FEEL_SKILL_X,
                        GameConstants::Colors::UILayout::HEART_FEEL_COUNT_OFFSET,
                        GameConstants::Colors::UILayout::HEART_FEEL_INSTRUCTION_OFFSET);
    
    renderSkillUI(config, hasSkill, false, remainingUses, maxUses);
}

void UIRenderer::renderFreeCameraUI(bool hasSkill, bool isActive, float timer, int remainingUses, int maxUses) {
    if (!hasSkill) return;
    
    SkillUIConfig config("FREE CAMERA", "PRESS C", 
                        GameConstants::Colors::UILayout::FREE_CAMERA_SKILL_X,
                        GameConstants::Colors::UILayout::FREE_CAMERA_COUNT_OFFSET,
                        GameConstants::Colors::UILayout::FREE_CAMERA_INSTRUCTION_OFFSET,
                        true, "FREE CAMERA ACTIVE", glm::vec2(0, 0)); // アクティブ状態あり
    
    renderSkillUI(config, hasSkill, isActive, remainingUses, maxUses);
}

void UIRenderer::renderBurstJumpUI(bool hasSkill, bool isActive, int remainingUses, int maxUses) {
    if (!hasSkill) return;
    
    SkillUIConfig config("BURST JUMP", "PRESS B", 
                        GameConstants::Colors::UILayout::BURST_JUMP_SKILL_X,
                        GameConstants::Colors::UILayout::BURST_JUMP_COUNT_OFFSET,
                        GameConstants::Colors::UILayout::BURST_JUMP_INSTRUCTION_OFFSET,
                        true, "PRESS SPACE!", 
                        glm::vec2(GameConstants::Colors::UILayout::BURST_JUMP_ACTIVE_X, 
                                 GameConstants::Colors::UILayout::BURST_JUMP_ACTIVE_Y)); // アクティブ状態あり
    
    renderSkillUI(config, hasSkill, isActive, remainingUses, maxUses);
}

void UIRenderer::renderStar(const glm::vec2& position, const glm::vec3& color, float scale) {
    float scaleX = static_cast<float>(windowWidth) / 1280.0f;
    float scaleY = static_cast<float>(windowHeight) / 720.0f;
    glm::vec2 scaledPosition = glm::vec2(position.x * scaleX, position.y * scaleY);
    float scaledScale = scale * std::min(scaleX, scaleY);
    
    glColor3f(color.r, color.g, color.b);
    
    float centerX = scaledPosition.x;
    float centerY = scaledPosition.y;
    
    glBegin(GL_TRIANGLES);
    for (int i = 0; i < 5; i++) {
        float angle1 = i * 72.0f * 3.14159f / 180.0f;
        float angle2 = (i + 2) * 72.0f * 3.14159f / 180.0f;
        
        float x1 = centerX + cos(angle1) * 12.0f * scale;
        float y1 = centerY + sin(angle1) * 12.0f * scale;
        
        float x2 = centerX + cos(angle1 + 36.0f * 3.14159f / 180.0f) * 5.0f * scale;
        float y2 = centerY + sin(angle1 + 36.0f * 3.14159f / 180.0f) * 5.0f * scale;
        
        float x3 = centerX + cos(angle2) * 12.0f * scale;
        float y3 = centerY + sin(angle2) * 12.0f * scale;
        
        glVertex2f(centerX, centerY);  // 中心点
        glVertex2f(x1, y1);            // 外側の点1
        glVertex2f(x2, y2);            // 内側の点
        
        glVertex2f(centerX, centerY);  // 中心点
        glVertex2f(x2, y2);            // 内側の点
        glVertex2f(x3, y3);            // 外側の点2
    }
    glEnd();
}

void UIRenderer::renderHeart(const glm::vec2& position, const glm::vec3& color, float scale) {
    float scaleX = static_cast<float>(windowWidth) / 1280.0f;
    float scaleY = static_cast<float>(windowHeight) / 720.0f;
    glm::vec2 scaledPosition = glm::vec2(position.x * scaleX, position.y * scaleY);
    float scaledScale = scale * std::min(scaleX, scaleY);
    
    glColor3f(color.r, color.g, color.b);
    
    float centerX = scaledPosition.x;
    float centerY = scaledPosition.y;
    
    glBegin(GL_TRIANGLES);
    
    for (int i = 0; i < 8; i++) {
        float angle1 = i * 45.0f * 3.14159f / 180.0f;
        float angle2 = (i + 1) * 45.0f * 3.14159f / 180.0f;
        
        float x1 = centerX - 8.0f * scale + cos(angle1) * 8.0f * scale;
        float y1 = centerY - 4.0f * scale + sin(angle1) * 8.0f * scale;
        float x2 = centerX - 8.0f * scale + cos(angle2) * 8.0f * scale;
        float y2 = centerY - 4.0f * scale + sin(angle2) * 8.0f * scale;
        
        glVertex2f(centerX - 8.0f * scale, centerY - 4.0f * scale); // 中心
        glVertex2f(x1, y1);
        glVertex2f(x2, y2);
    }
    
    for (int i = 0; i < 8; i++) {
        float angle1 = i * 45.0f * 3.14159f / 180.0f;
        float angle2 = (i + 1) * 45.0f * 3.14159f / 180.0f;
        
        float x1 = centerX + 8.0f * scale + cos(angle1) * 8.0f * scale;
        float y1 = centerY - 4.0f * scale + sin(angle1) * 8.0f * scale;
        float x2 = centerX + 8.0f * scale + cos(angle2) * 8.0f * scale;
        float y2 = centerY - 4.0f * scale + sin(angle2) * 8.0f * scale;
        
        glVertex2f(centerX + 8.0f * scale, centerY - 4.0f * scale); // 中心
        glVertex2f(x1, y1);
        glVertex2f(x2, y2);
    }
    
    glVertex2f(centerX, centerY + 12.0f * scale); // 下部の尖った部分
    glVertex2f(centerX - 16.0f * scale, centerY - 4.0f * scale); // 左側
    glVertex2f(centerX + 16.0f * scale, centerY - 4.0f * scale); // 右側
    
    glEnd();
}

} // namespace gfx
