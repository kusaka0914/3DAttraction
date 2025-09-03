#include "ui_renderer.h"
#include "bitmap_font.h"
#include <iostream>

namespace gfx {

UIRenderer::UIRenderer() {
    font.initialize(); // フォントを初期化
}

UIRenderer::~UIRenderer() {
}

void UIRenderer::begin2DMode() {
    // 2D描画モードに切り替え
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, 1280, 720, 0, -1, 1);
    
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    
    glDisable(GL_DEPTH_TEST);
}

void UIRenderer::end2DMode() {
    // 3D描画モードに戻す
    glEnable(GL_DEPTH_TEST);
    
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

void UIRenderer::renderText(const std::string& text, const glm::vec2& position, const glm::vec3& color, float scale) {
    begin2DMode();
    
    glColor3f(color.r, color.g, color.b);
    
    float currentX = position.x;
    float charWidth = GameConstants::RenderConstants::CHAR_WIDTH * scale;
    float charHeight = GameConstants::RenderConstants::CHAR_HEIGHT * scale;
    float spaceWidth = GameConstants::RenderConstants::SPACE_WIDTH * scale;
    
    for (size_t i = 0; i < text.length(); i++) {
        char c = text[i];
        
        if (c == ' ') {
            currentX += spaceWidth;
            continue;
        }
        
        renderBitmapChar(c, glm::vec2(currentX, position.y), color, scale);
        currentX += charWidth + GameConstants::RenderConstants::CHAR_SPACING * scale;  // 文字間隔を増加
    }
    
    end2DMode();
}

void UIRenderer::renderBitmapChar(char c, const glm::vec2& position, const glm::vec3& color, float scale) {
    if (!font.hasCharacter(c)) {
        return;
    }
    
    const std::vector<bool>& charData = font.getCharacter(c);
    
    glColor3f(color.r, color.g, color.b);
    
    // 8x12のビットマップを描画
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

// 統合されたUI描画関数
void UIRenderer::renderGameUI(const GameUIState& state) {
    begin2DMode();
    
    // 時間表示
    if (state.showTime) {
        renderTimeDisplay(state.remainingTime, state.timeLimit);
    }
    
    // ゴール表示
    if (state.showGoal) {
        renderGoalDisplay(state.timeLimit);
    }
    
    // 星表示
    if (state.showStars) {
        renderStarsDisplay(state.existingStars);
    }
    
    // ライフ表示
    if (state.showLives) {
        renderLivesDisplay(state.lives);
    }
    
    end2DMode();
}

void UIRenderer::renderTimeUI(float remainingTime, float timeLimit, int earnedStars, int existingStars, int lives) {
    GameUIState state;
    state.showTime = true;
    state.showGoal = true;
    state.showStars = true;
    state.showLives = true;
    state.remainingTime = remainingTime;
    state.timeLimit = timeLimit;
    state.existingStars = existingStars;
    state.lives = lives;
    
    renderGameUI(state);
}

// 時間表示の描画
void UIRenderer::renderTimeDisplay(float remainingTime, float timeLimit) {
    std::string timeText = std::to_string(static_cast<int>(remainingTime)) + "s";
    glm::vec3 timeColor = (remainingTime <= 5.0f) ? GameConstants::UI_WARNING_COLOR : GameConstants::UI_TEXT_COLOR;
    
    renderText(timeText, glm::vec2(GameConstants::Colors::UILayout::TIME_UI_X, 
                                   GameConstants::Colors::UILayout::TIME_UI_Y), timeColor, GameConstants::Colors::UILayout::TIME_UI_SCALE);
}

// ゴール表示の描画
void UIRenderer::renderGoalDisplay(float timeLimit) {
    std::string goalText = "GOAL";
    renderText(goalText, glm::vec2(GameConstants::Colors::UILayout::GOAL_UI_X, 
                                   GameConstants::Colors::UILayout::GOAL_UI_Y), GameConstants::UI_TEXT_COLOR, GameConstants::Colors::UILayout::GOAL_UI_SCALE);
    
    if (timeLimit <= 20) {
        renderText("5s", glm::vec2(GameConstants::Colors::UILayout::GOAL_TIME_5S_X, 
                                   GameConstants::Colors::UILayout::GOAL_UI_Y), GameConstants::UI_TEXT_COLOR, GameConstants::Colors::UILayout::GOAL_UI_SCALE);
        renderText("10s", glm::vec2(GameConstants::Colors::UILayout::GOAL_TIME_10S_X, 
                                    GameConstants::Colors::UILayout::GOAL_UI_Y), GameConstants::UI_TEXT_COLOR, GameConstants::Colors::UILayout::GOAL_UI_SCALE);
    } else {
        renderText("10s", glm::vec2(GameConstants::Colors::UILayout::GOAL_TIME_5S_X, 
                                    GameConstants::Colors::UILayout::GOAL_UI_Y), GameConstants::UI_TEXT_COLOR, GameConstants::Colors::UILayout::GOAL_UI_SCALE);
        renderText("20s", glm::vec2(GameConstants::Colors::UILayout::GOAL_TIME_20S_X, 
                                    GameConstants::Colors::UILayout::GOAL_UI_Y), GameConstants::UI_TEXT_COLOR, GameConstants::Colors::UILayout::GOAL_UI_SCALE);
    }
}

// 星表示の描画
void UIRenderer::renderStarsDisplay(int existingStars) {
    for (int i = 0; i < 3; i++) {
        glm::vec2 starPos = glm::vec2(GameConstants::Colors::UILayout::STARS_START_X + 
                                      i * GameConstants::Colors::UILayout::STARS_SPACING, 
                                      GameConstants::Colors::UILayout::STARS_Y);
        glm::vec3 starColor = (i < existingStars) ? GameConstants::STAR_ACTIVE : GameConstants::STAR_INACTIVE;
        renderStar(starPos, starColor, GameConstants::Colors::UILayout::STARS_SCALE);
    }
}

// ライフ表示の描画
void UIRenderer::renderLivesDisplay(int lives) {
    for (int i = 0; i < 6; i++) {
        glm::vec3 heartColor = (i < lives) ? GameConstants::LIFE_ACTIVE : GameConstants::LIFE_INACTIVE;
        float heartX = GameConstants::Colors::UILayout::HEART_START_X + i * GameConstants::Colors::UILayout::HEART_SPACING;
        renderHeart(glm::vec2(heartX, GameConstants::Colors::UILayout::HEART_Y), heartColor, GameConstants::Colors::UILayout::HEART_SCALE);
    }
}

// 説明テキストの共通化関数
void UIRenderer::renderExplanationText(const std::string& type, const glm::vec2& position) {
    if (type == "lives") {
        std::string explanation1 = "THESE ARE YOUR LIVES !";
        std::string explanation2 = "THEY DECREASE AS YOU FALL !";
        std::string explanation3 = "IF THEY'RE ALL GONE, IT'S GAME OVER !";
        std::string explanation4 = "SO BE CAREFUL !";

        int offsetY = position.y - 100;
        int interval = 40;
        
        renderText(explanation1, glm::vec2(position.x - 460, offsetY), GameConstants::UI_TEXT_COLOR, GameConstants::Colors::UILayout::EXPLANATION_SCALE);
        renderText(explanation2, glm::vec2(position.x - 480, offsetY + interval), GameConstants::UI_TEXT_COLOR, GameConstants::Colors::UILayout::EXPLANATION_SCALE);
        renderText(explanation3, glm::vec2(position.x - 520, offsetY + interval * 2), GameConstants::UI_TEXT_COLOR, GameConstants::Colors::UILayout::EXPLANATION_SCALE);
        renderText(explanation4, glm::vec2(position.x - 420, offsetY + interval * 3), GameConstants::UI_TEXT_COLOR, GameConstants::Colors::UILayout::EXPLANATION_SCALE);
    }
    else if (type == "time") {
        std::string timeLimitText = "THIS IS THE TIME LIMIT!";
        std::string timeLimitText2 = "IF IT REACHES 0, THE GAME IS OVER!";
        std::string timeLimitText3 = "AIM FOR THE GOAL WITHIN THE TIME LIMIT!";

        int offsetY = position.y;
        int interval = 40;
        
        renderText(timeLimitText, glm::vec2(position.x, offsetY), GameConstants::UI_TEXT_COLOR, GameConstants::Colors::UILayout::EXPLANATION_SCALE);
        renderText(timeLimitText2, glm::vec2(position.x, offsetY + interval), GameConstants::UI_TEXT_COLOR, GameConstants::Colors::UILayout::EXPLANATION_SCALE);
        renderText(timeLimitText3, glm::vec2(position.x, offsetY + interval * 2), GameConstants::UI_TEXT_COLOR, GameConstants::Colors::UILayout::EXPLANATION_SCALE);
    }
    else if (type == "stars") {
        std::string starsText = "THESE ARE STARS !";
        std::string starsText2 = "YOU CAN GET THEM BY CLEARING A STAGE !";
        std::string starsText3 = "THE MORE TIME YOU HAVE LEFT, THE MORE STARS YOU'LL GET !";
        std::string starsText4 = "CLEAR THE STAGE QUICKLY TO COLLECT AS MANY STARS AS POSSIBLE !";

        int offsetY = position.y;
        int interval = 40;
        
        renderText(starsText, glm::vec2(position.x, offsetY), GameConstants::UI_TEXT_COLOR, GameConstants::Colors::UILayout::EXPLANATION_SCALE);
        renderText(starsText2, glm::vec2(position.x, offsetY + interval), GameConstants::UI_TEXT_COLOR, GameConstants::Colors::UILayout::EXPLANATION_SCALE);
        renderText(starsText3, glm::vec2(position.x, offsetY + interval * 2), GameConstants::UI_TEXT_COLOR, GameConstants::Colors::UILayout::EXPLANATION_SCALE);
        renderText(starsText4, glm::vec2(position.x, offsetY + interval * 3), GameConstants::UI_TEXT_COLOR, GameConstants::Colors::UILayout::EXPLANATION_SCALE);
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
    
    // 説明テキストのみ追加
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
    
    // 説明テキストのみ追加
    renderExplanationText("time", glm::vec2(800, 120));
}

void UIRenderer::renderLivesTimeAndStarsUI(int lives, float remainingTime, float timeLimit, int earnedStars, int existingStars) {
    GameUIState state;
    state.showTime = true;
    state.showGoal = true;
    state.showStars = true;
    state.showLives = true;
    state.remainingTime = remainingTime;
    state.timeLimit = timeLimit;
    state.existingStars = existingStars;
    state.lives = lives;
    
    renderGameUI(state);
    
    // 説明テキストのみ追加
    renderExplanationText("stars", glm::vec2(590, 100));
}

// 統合されたスキルUI描画関数
void UIRenderer::renderSkillUI(const SkillUIConfig& config, bool hasSkill, bool isActive, 
                               int remainingUses, int maxUses, bool isEasyMode) {
    if (!hasSkill) return;
    
    begin2DMode();
    
    std::string skillText = config.skillName;
    glm::vec3 skillColor = GameConstants::UI_TEXT_COLOR;
    glm::vec3 usesColor = GameConstants::UI_TEXT_COLOR;
    
    // 使用回数が0の場合は灰色
    if (remainingUses <= 0) {
        usesColor = GameConstants::UI_DISABLED_COLOR;
        skillColor = GameConstants::UI_DISABLED_COLOR;
    }
    
    // アクティブ状態の場合は青色
    if (isActive) {
        usesColor = GameConstants::UI_ACTIVE_COLOR;
        skillColor = GameConstants::UI_ACTIVE_COLOR;
    }
    
    // スキル名を描画
    renderText(skillText, glm::vec2(config.skillX, GameConstants::Colors::UILayout::SKILL_START_Y), skillColor, GameConstants::Colors::UILayout::SKILL_SCALE);
    
    // 使用回数を描画
    renderText(std::to_string(remainingUses), glm::vec2(config.skillX + config.countOffset, GameConstants::Colors::UILayout::SKILL_COUNT_Y), usesColor, GameConstants::Colors::UILayout::SKILL_COUNT_SCALE);
    
    // 操作説明を描画
    renderText(config.instructionText, glm::vec2(config.skillX + config.instructionOffset, GameConstants::Colors::UILayout::SKILL_INSTRUCTION_Y), usesColor, GameConstants::Colors::UILayout::SKILL_INSTRUCTION_SCALE);
    
    // アクティブ状態の特別表示
    if (config.hasActiveState && isActive) {
        renderText(config.activeText, config.activePosition, GameConstants::UI_ACTIVE_COLOR, GameConstants::Colors::UILayout::SKILL_ACTIVE_SCALE);
    }
    
    end2DMode();
}

// 個別スキルUI描画関数（後方互換性のため）
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
    glColor3f(color.r, color.g, color.b);
    
    // 星の中心点
    float centerX = position.x;
    float centerY = position.y;
    
    // 星の5つの角を描画（塗りつぶし）
    glBegin(GL_TRIANGLES);
    for (int i = 0; i < 5; i++) {
        float angle1 = i * 72.0f * 3.14159f / 180.0f;
        float angle2 = (i + 2) * 72.0f * 3.14159f / 180.0f;
        
        // 外側の点
        float x1 = centerX + cos(angle1) * 12.0f * scale;
        float y1 = centerY + sin(angle1) * 12.0f * scale;
        
        // 内側の点
        float x2 = centerX + cos(angle1 + 36.0f * 3.14159f / 180.0f) * 5.0f * scale;
        float y2 = centerY + sin(angle1 + 36.0f * 3.14159f / 180.0f) * 5.0f * scale;
        
        // 次の外側の点
        float x3 = centerX + cos(angle2) * 12.0f * scale;
        float y3 = centerY + sin(angle2) * 12.0f * scale;
        
        // 中心から各点への三角形を描画（塗りつぶし）
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
    glColor3f(color.r, color.g, color.b);
    
    // ハートの中心点
    float centerX = position.x;
    float centerY = position.y;
    
    // ハートの形状を描画（塗りつぶし）
    glBegin(GL_TRIANGLES);
    
    // 左側の円形部分
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
    
    // 右側の円形部分
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
    
    // 下部の三角形部分
    glVertex2f(centerX, centerY + 12.0f * scale); // 下部の尖った部分
    glVertex2f(centerX - 16.0f * scale, centerY - 4.0f * scale); // 左側
    glVertex2f(centerX + 16.0f * scale, centerY - 4.0f * scale); // 右側
    
    glEnd();
}

} // namespace gfx
