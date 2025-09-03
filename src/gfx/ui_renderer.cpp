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
    
    // 深度テストを無効化（UI表示のため）
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
    // 2D描画モードに切り替え
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, 1280, 720, 0, -1, 1);
    
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    
    // 深度テストを無効化（UI表示のため）
    glDisable(GL_DEPTH_TEST);
    
    // 色を設定
    glColor3f(color.r, color.g, color.b);
    
    float currentX = position.x;
    float charWidth = GameConstants::RenderConstants::CHAR_WIDTH * scale;
    float charHeight = GameConstants::RenderConstants::CHAR_HEIGHT * scale;
    float spaceWidth = GameConstants::RenderConstants::SPACE_WIDTH * scale;  // スペース幅を増加
    
    for (size_t i = 0; i < text.length(); i++) {
        char c = text[i];
        
        if (c == ' ') {
            currentX += spaceWidth;
            continue;
        }
        
        // ビットマップフォントで文字を描画
        renderBitmapChar(c, glm::vec2(currentX, position.y), color, scale);
        currentX += charWidth + GameConstants::RenderConstants::CHAR_SPACING * scale;  // 文字間隔を増加
    }
    
    // 3D描画モードに戻す
    glEnable(GL_DEPTH_TEST);
    
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

void UIRenderer::renderBitmapChar(char c, const glm::vec2& position, const glm::vec3& color, float scale) {
    if (!font.hasCharacter(c)) {
        // 文字が見つからない場合は空白として扱う
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

void UIRenderer::renderTimeUI(float remainingTime, float timeLimit, int earnedStars, int existingStars, int lives) {
    begin2DMode();
    
    // 残り時間表示（右上、より見やすい位置に調整）
    std::string timeText = std::to_string(static_cast<int>(remainingTime)) + "s";
    glm::vec3 timeColor = GameConstants::UI_TEXT_COLOR;
    
    // 時間が少なくなったら赤色で警告
    if (remainingTime <= 5.0f) {
        timeColor = GameConstants::UI_WARNING_COLOR;
    }
    
    renderText(timeText, glm::vec2(GameConstants::RenderConstants::UILayout::TIME_UI_X, 
                                   GameConstants::RenderConstants::UILayout::TIME_UI_Y), timeColor, 3.0f);
    
    std::string goalText = "GOAL";
    glm::vec3 goalColor = GameConstants::UI_TEXT_COLOR;
    
    renderText(goalText, glm::vec2(GameConstants::RenderConstants::UILayout::GOAL_UI_X, 
                                   GameConstants::RenderConstants::UILayout::GOAL_UI_Y), goalColor, 1.0f);
    
    if(timeLimit <= 20) {
        std::string goalText2 = "5s";
        glm::vec3 goalColor2 = glm::vec3(1.0f, 1.0f, 1.0f);
        renderText(goalText2, glm::vec2(1040, 65), goalColor2, 1.0f);
        std::string goalText3 = "10s";
        glm::vec3 goalColor3 = glm::vec3(1.0f, 1.0f, 1.0f);
        renderText(goalText3, glm::vec2(1110, 65), goalColor3, 1.0f);
    }

    if(timeLimit > 20) {
        std::string goalText2 = "10s";
        glm::vec3 goalColor2 = glm::vec3(1.0f, 1.0f, 1.0f);
        renderText(goalText2, glm::vec2(1040, 65), goalColor2, 1.0f);
        std::string goalText3 = "20s";
        glm::vec3 goalColor3 = glm::vec3(1.0f, 1.0f, 1.0f);
        renderText(goalText3, glm::vec2(1110, 65), goalColor3, 1.0f);
    }

    if(existingStars == 0) {
        for (int i = 0; i < 3; i++) {
            glm::vec2 starPos = glm::vec2(GameConstants::RenderConstants::UILayout::RIGHT_UI_X + 
                                          i * GameConstants::RenderConstants::UILayout::STAR_SPACING, 
                                          GameConstants::RenderConstants::UILayout::STAR_UI_Y);
            glm::vec3 starColor = GameConstants::STAR_INACTIVE;
            renderStar(starPos, starColor, 1.5f);
        }
    } else {
        // 星の表示（右上、時間の下）
        for (int i = 0; i < existingStars; i++) {
            glm::vec2 starPos = glm::vec2(GameConstants::RenderConstants::UILayout::RIGHT_UI_X + 
                                          i * GameConstants::RenderConstants::UILayout::STAR_SPACING, 
                                          GameConstants::RenderConstants::UILayout::STAR_UI_Y);
            glm::vec3 starColor = GameConstants::STAR_ACTIVE;
            renderStar(starPos, starColor, 1.5f);
        }
        
        for (int i = existingStars; i < 3; i++) {
            glm::vec2 starPos = glm::vec2(GameConstants::RenderConstants::UILayout::RIGHT_UI_X + 
                                          i * GameConstants::RenderConstants::UILayout::STAR_SPACING, 
                                          GameConstants::RenderConstants::UILayout::STAR_UI_Y);
            glm::vec3 starColor = GameConstants::STAR_INACTIVE;
            renderStar(starPos, starColor, 1.5f);
        }
    }
    
    // ハートを6個表示（右から消えていく）
    for (int i = 0; i < 6; i++) {
        glm::vec3 heartColor;
        if (i < lives) {
            heartColor = GameConstants::LIFE_ACTIVE; // 赤色（残っているライフ）
        } else {
            heartColor = GameConstants::LIFE_INACTIVE; // 灰色（失ったライフ）
        }
        
        // STAGEテキストの右側にハートを配置
        float heartX = GameConstants::RenderConstants::UILayout::LEFT_UI_X + 
                      i * GameConstants::RenderConstants::UILayout::LIFE_SPACING;
        float heartY = GameConstants::RenderConstants::UILayout::LEFT_UI_Y;
        
        renderHeart(glm::vec2(heartX, heartY), heartColor, 1.0f);
    }
    
    end2DMode();
}

void UIRenderer::renderLivesOnly(int lives) {
    begin2DMode();
    
    // ハートを6個表示（右から消えていく）
    for (int i = 0; i < 6; i++) {
        glm::vec3 heartColor;
        if (i < lives) {
            heartColor = glm::vec3(1.0f, 0.3f, 0.3f); // 赤色（残っているライフ）
        } else {
            heartColor = glm::vec3(0.3f, 0.3f, 0.3f); // 灰色（失ったライフ）
        }
        
        // STAGEテキストの右側にハートを配置
        float heartX = 200.0f + i * 40.0f;
        float heartY = 45.0f;
        
        renderHeart(glm::vec2(heartX, heartY), heartColor, 1.0f);
    }
    
    end2DMode();
}

void UIRenderer::renderTimeUIOnly(float remainingTime, float timeLimit, int earnedStars, int existingStars) {
    begin2DMode();
    
    // 残り時間表示（右上、より見やすい位置に調整）
    std::string timeText = std::to_string(static_cast<int>(remainingTime)) + "s";
    glm::vec3 timeColor = glm::vec3(1.0f, 1.0f, 1.0f);
    
    // 時間が少なくなったら赤色で警告
    if (remainingTime <= 5.0f) {
        timeColor = glm::vec3(1.0f, 0.0f, 0.0f);
    }
    
    renderText(timeText, glm::vec2(1170, 30), timeColor, 3.0f);
    std::string goalText = "GOAL";
    glm::vec3 goalColor = glm::vec3(1.0f, 1.0f, 1.0f);
    
    renderText(goalText, glm::vec2(962, 65), goalColor, 1.0f);
    
    if(timeLimit <= 20) {
        std::string goalText2 = "5s";
        glm::vec3 goalColor2 = glm::vec3(1.0f, 1.0f, 1.0f);
        renderText(goalText2, glm::vec2(1040, 65), goalColor2, 1.0f);
        std::string goalText3 = "10s";
        glm::vec3 goalColor3 = glm::vec3(1.0f, 1.0f, 1.0f);
        renderText(goalText3, glm::vec2(1110, 65), goalColor3, 1.0f);
    }

    if(timeLimit > 20) {
        std::string goalText2 = "10s";
        glm::vec3 goalColor2 = glm::vec3(1.0f, 1.0f, 1.0f);
        renderText(goalText2, glm::vec2(1040, 65), goalColor2, 1.0f);
        std::string goalText3 = "20s";
        glm::vec3 goalColor3 = glm::vec3(1.0f, 1.0f, 1.0f);
        renderText(goalText3, glm::vec2(1110, 65), goalColor3, 1.0f);
    }

    if(existingStars == 0) {
        for (int i = 0; i < 3; i++) {
            glm::vec2 starPos = glm::vec2(GameConstants::RenderConstants::UILayout::RIGHT_UI_X + 
                                          i * GameConstants::RenderConstants::UILayout::STAR_SPACING, 
                                          GameConstants::RenderConstants::UILayout::STAR_UI_Y);
            glm::vec3 starColor = GameConstants::STAR_INACTIVE;
            renderStar(starPos, starColor, 1.5f);
        }
    } else {
        // 星の表示（右上、時間の下）
        for (int i = 0; i < existingStars; i++) {
            glm::vec2 starPos = glm::vec2(GameConstants::RenderConstants::UILayout::RIGHT_UI_X + 
                                          i * GameConstants::RenderConstants::UILayout::STAR_SPACING, 
                                          GameConstants::RenderConstants::UILayout::STAR_UI_Y);
            glm::vec3 starColor = GameConstants::STAR_ACTIVE;
            renderStar(starPos, starColor, 1.5f);
        }
        
        for (int i = existingStars; i < 3; i++) {
            glm::vec2 starPos = glm::vec2(GameConstants::RenderConstants::UILayout::RIGHT_UI_X + 
                                          i * GameConstants::RenderConstants::UILayout::STAR_SPACING, 
                                          GameConstants::RenderConstants::UILayout::STAR_UI_Y);
            glm::vec3 starColor = GameConstants::STAR_INACTIVE;
            renderStar(starPos, starColor, 1.5f);
        }
    }
    
    end2DMode();
}

void UIRenderer::renderLivesWithExplanation(int lives) {
    begin2DMode();
    
    // ハートを6個表示（右から消えていく）
    for (int i = 0; i < 6; i++) {
        glm::vec3 heartColor;
        if (i < lives) {
            heartColor = glm::vec3(1.0f, 0.3f, 0.3f); // 赤色（残っているライフ）
        } else {
            heartColor = glm::vec3(0.3f, 0.3f, 0.3f); // 灰色（失ったライフ）
        }
        
        // STAGEテキストの右側にハートを配置
        float heartX = 200.0f + i * 40.0f;
        float heartY = 45.0f;
        
        renderHeart(glm::vec2(heartX, heartY), heartColor, 1.0f);
    }
    
    // 説明テキストを表示（ライフのUIの下）
    std::string explanation1 = "THESE ARE YOUR LIVES !";
    std::string explanation2 = "THEY DECREASE AS YOU FALL !";
    std::string explanation3 = "IF THEY'RE ALL GONE, IT'S GAME OVER !";
    std::string explanation4 = "SO BE CAREFUL !";
    
    // 中央に配置（ライフのUIの下）
    float centerX = 640.0f;  // 1280/2
    float centerY = 200.0f;  // ライフのUIの下に配置
    
    renderText(explanation1, glm::vec2(centerX - 460, centerY-100), glm::vec3(1.0f, 1.0f, 1.0f), 1.2f);
    renderText(explanation2, glm::vec2(centerX - 480, centerY -60), glm::vec3(1.0f, 1.0f, 1.0f), 1.2f);
    renderText(explanation3, glm::vec2(centerX - 520, centerY - 20), glm::vec3(1.0f, 1.0f, 1.0f), 1.2f);
    renderText(explanation4, glm::vec2(centerX - 420, centerY + 20), glm::vec3(1.0f, 1.0f, 1.0f), 1.2f);
    
    end2DMode();
}

void UIRenderer::renderLivesAndTimeUI(int lives, float remainingTime, float timeLimit, int earnedStars, int existingStars) {
    begin2DMode();
    
    // 残り時間表示（右上、より見やすい位置に調整）
    std::string timeText = std::to_string(static_cast<int>(remainingTime)) + "s";
    glm::vec3 timeColor = GameConstants::UI_TEXT_COLOR;
    
    // 時間が少なくなったら赤色で警告
    if (remainingTime <= 5.0f) {
        timeColor = GameConstants::UI_WARNING_COLOR;
    }
    
    renderText(timeText, glm::vec2(GameConstants::RenderConstants::UILayout::TIME_UI_X, 
                                   GameConstants::RenderConstants::UILayout::TIME_UI_Y), timeColor, 3.0f);
    
    glm::vec3 timeLimitColor = GameConstants::UI_TEXT_COLOR;
    std::string timeLimitText = "THIS IS THE TIME LIMIT!";
    std::string timeLimitText2 = "IF IT REACHES 0, THE GAME IS OVER!";
    std::string timeLimitText3 = "AIM FOR THE GOAL WITHIN THE TIME LIMIT!";

    renderText(timeLimitText, glm::vec2(800, 120), timeLimitColor, 1.2f);
    renderText(timeLimitText2, glm::vec2(800, 170), timeLimitColor, 1.2f);
    renderText(timeLimitText3, glm::vec2(800, 220), timeLimitColor, 1.2f);
    
    // ハートを6個表示（右から消えていく）
    for (int i = 0; i < 6; i++) {
        glm::vec3 heartColor;
        if (i < lives) {
            heartColor = GameConstants::LIFE_ACTIVE; // 赤色（残っているライフ）
        } else {
            heartColor = GameConstants::LIFE_INACTIVE; // 灰色（失ったライフ）
        }
        
        // STAGEテキストの右側にハートを配置
        float heartX = GameConstants::RenderConstants::UILayout::LEFT_UI_X + 
                      i * GameConstants::RenderConstants::UILayout::LIFE_SPACING;
        float heartY = GameConstants::RenderConstants::UILayout::LEFT_UI_Y;
        
        renderHeart(glm::vec2(heartX, heartY), heartColor, 1.0f);
    }
    
    end2DMode();
}

void UIRenderer::renderLivesTimeAndStarsUI(int lives, float remainingTime, float timeLimit, int earnedStars, int existingStars) {
    begin2DMode();
    
    // 残り時間表示（右上、より見やすい位置に調整）
    std::string timeText = std::to_string(static_cast<int>(remainingTime)) + "s";
    glm::vec3 timeColor = glm::vec3(1.0f, 1.0f, 1.0f);
    
    // 時間が少なくなったら赤色で警告
    if (remainingTime <= 5.0f) {
        timeColor = glm::vec3(1.0f, 0.0f, 0.0f);
    }
    
    renderText(timeText, glm::vec2(1170, 30), timeColor, 3.0f);
    std::string goalText = "GOAL";
    glm::vec3 goalColor = glm::vec3(1.0f, 1.0f, 1.0f);
    
    renderText(goalText, glm::vec2(962, 65), goalColor, 1.0f);
    
    if(timeLimit <= 20) {
        std::string goalText2 = "5s";
        glm::vec3 goalColor2 = glm::vec3(1.0f, 1.0f, 1.0f);
        renderText(goalText2, glm::vec2(1040, 65), goalColor2, 1.0f);
        std::string goalText3 = "10s";
        glm::vec3 goalColor3 = glm::vec3(1.0f, 1.0f, 1.0f);
        renderText(goalText3, glm::vec2(1110, 65), goalColor3, 1.0f);
    }

    if(timeLimit > 20) {
        std::string goalText2 = "10s";
        glm::vec3 goalColor2 = glm::vec3(1.0f, 1.0f, 1.0f);
        renderText(goalText2, glm::vec2(1040, 65), goalColor2, 1.0f);
        std::string goalText3 = "20s";
        glm::vec3 goalColor3 = glm::vec3(1.0f, 1.0f, 1.0f);
        renderText(goalText3, glm::vec2(1110, 65), goalColor3, 1.0f);
    }

    if(existingStars == 0) {
        for (int i = 0; i < 3; i++) {
            glm::vec2 starPos = glm::vec2(980 + i * 70, 40);
            glm::vec3 starColor = glm::vec3(0.5f, 0.5f, 0.5f);
            renderStar(starPos, starColor, 1.5f);
        }
    } else {
        // 星の表示（右上、時間の下）
        for (int i = 0; i < existingStars; i++) {
            glm::vec2 starPos = glm::vec2(980 + i * 70, 40);
            glm::vec3 starColor = GameConstants::STAR_ACTIVE;
            renderStar(starPos, starColor, 1.5f);
        }
        
        for (int i = existingStars; i < 3; i++) {
            glm::vec2 starPos = glm::vec2(980 + i * 70, 40);
            glm::vec3 starColor = glm::vec3(0.5f, 0.5f, 0.5f);
            renderStar(starPos, starColor, 1.5f);
        }
    }
    
    // ハートを6個表示（右から消えていく）
    for (int i = 0; i < 6; i++) {
        glm::vec3 heartColor;
        if (i < lives) {
            heartColor = glm::vec3(1.0f, 0.3f, 0.3f); // 赤色（残っているライフ）
        } else {
            heartColor = glm::vec3(0.3f, 0.3f, 0.3f); // 灰色（失ったライフ）
        }
        
        // STAGEテキストの右側にハートを配置
        float heartX = 200.0f + i * 40.0f;
        float heartY = 45.0f;
        
        renderHeart(glm::vec2(heartX, heartY), heartColor, 1.0f);
    }

    std::string starsText = "THESE ARE STARS !";
    std::string starsText2 = "YOU CAN GET THEM BY CLEARING A STAGE !";
    std::string starsText3 = "THE MORE TIME YOU HAVE LEFT, THE MORE STARS YOU'LL GET !";
    std::string starsText4 = "CLEAR THE STAGE QUICKLY TO COLLECT AS MANY STARS AS POSSIBLE !";
    glm::vec3 starsColor = glm::vec3(1.0f, 1.0f, 1.0f);
    renderText(starsText, glm::vec2(590, 100), starsColor, 1.2f);
    renderText(starsText2, glm::vec2(590, 150), starsColor, 1.2f);
    renderText(starsText3, glm::vec2(590, 200), starsColor, 1.2f);
    renderText(starsText4, glm::vec2(590, 250), starsColor, 1.2f);
    
    end2DMode();
}

void UIRenderer::renderTimeStopUI(bool hasSkill, bool isTimeStopped, float timeStopTimer, int remainingUses, int maxUses) {
    // スキルを取得していない場合は表示しない
    if (!hasSkill) {
        return;
    }
    
    begin2DMode();
    
    // 時間停止スキルの表示（左下）
    std::string skillText = "TIME STOP";
    glm::vec3 skillColor = glm::vec3(1.0f, 1.0f, 1.0f);
    
    // 使用回数表示
    std::string usesText = "PRESS Q";
    glm::vec3 usesColor = glm::vec3(1.0f, 1.0f, 1.0f);
    if (remainingUses <= 0) {
        usesColor = glm::vec3(0.5f, 0.5f, 0.5f);
        skillColor = glm::vec3(0.5f, 0.5f, 0.5f);
    }
    // 時間停止中は明るい青色
    if (isTimeStopped) {
        skillColor = glm::vec3(0.5f, 0.5f, 1.0f);
        usesColor = glm::vec3(0.5f, 0.5f, 1.0f);
    }
    std::string countText = std::to_string(remainingUses);
    renderText(skillText, glm::vec2(30, 650), skillColor, 1.5f);
    renderText(countText, glm::vec2(80, 600), usesColor, 3.0f);
    
    renderText(usesText, glm::vec2(50, 680), usesColor, 1.2f);
    
    end2DMode();
}

void UIRenderer::renderDoubleJumpUI(bool hasSkill, bool isEasyMode, int remainingUses, int maxUses) {
    // お助けモードの場合は表示しない（無制限のため）
    if (isEasyMode) {
        return;
    }
    
    // スキルを取得していない場合は表示しない
    if (!hasSkill) {
        return;
    }
    
    begin2DMode();
    
    // 二段ジャンプスキルの表示（左下、時間停止スキルの上）
    std::string skillText = "DOUBLE JUMP";
    glm::vec3 skillColor = glm::vec3(1.0f, 1.0f, 1.0f);
    
    // 使用回数表示
    std::string usesText = "PRESS SPACE IN AIR";
    glm::vec3 usesColor = glm::vec3(1.0f, 1.0f, 1.0f);
    
    // 使用回数が0の場合は灰色
    if (remainingUses <= 0) {
        usesColor = glm::vec3(0.5f, 0.5f, 0.5f);
        skillColor = glm::vec3(0.5f, 0.5f, 0.5f);
    }
    std::string countText = std::to_string(remainingUses);

    renderText(skillText, glm::vec2(230, 650), skillColor, 1.5f);
    renderText(countText, glm::vec2(300, 600), usesColor, 3.0f);
    renderText(usesText, glm::vec2(205, 680), usesColor, 1.2f);
    
    end2DMode();
}

void UIRenderer::renderHeartFeelUI(bool hasSkill, int remainingUses, int maxUses, int currentLives) {
    // スキルを取得していない場合は表示しない
    if (!hasSkill) {
        return;
    }
    
    begin2DMode();
    
    // ハートフエールスキルの表示（左下、二段ジャンプスキルの上）
    std::string skillText = "HEART FEEL";
    glm::vec3 skillColor = glm::vec3(1.0f, 1.0f, 1.0f);
    
    renderText(skillText, glm::vec2(460, 650), skillColor, 1.5f);
    
    // 使用回数表示
    std::string usesText = "PRESS H";
    glm::vec3 usesColor = glm::vec3(1.0f, 1.0f, 1.0f);
    
    // 使用回数が0の場合は灰色
    if (remainingUses <= 0) {
        usesColor = glm::vec3(0.5f, 0.5f, 0.5f);
        skillColor = glm::vec3(0.5f, 0.5f, 0.5f);
    }
    std::string countText = std::to_string(remainingUses);
    renderText(skillText, glm::vec2(460, 650), skillColor, 1.5f);
    renderText(countText, glm::vec2(520, 600), usesColor, 3.0f);
    renderText(usesText, glm::vec2(490, 680), usesColor, 1.2f);
    
    end2DMode();
}

void UIRenderer::renderFreeCameraUI(bool hasSkill, bool isActive, float timer, int remainingUses, int maxUses) {
    // スキルを取得していない場合は表示しない
    if (!hasSkill) {
        return;
    }
    
    begin2DMode();
    
    // フリーカメラスキルの表示（左下、ハートフエールスキルの上）
    std::string skillText = "FREE CAMERA";
    glm::vec3 skillColor = glm::vec3(1.0f, 1.0f, 1.0f);
    
    // 使用回数表示
    std::string usesText = "PRESS C";
    glm::vec3 usesColor = glm::vec3(1.0f, 1.0f, 1.0f);
    
    // 使用回数が0の場合は灰色
    if (remainingUses <= 0) {
        usesColor = glm::vec3(0.5f, 0.5f, 0.5f);
        skillColor = glm::vec3(0.5f, 0.5f, 0.5f);
    }
    std::string countText = std::to_string(remainingUses);
    
    // アクティブ中の残り時間表示
    if (isActive) {
        usesColor = glm::vec3(0.5f, 0.5f, 1.0f);
        skillColor = glm::vec3(0.5f, 0.5f, 1.0f);
    }

    renderText(skillText, glm::vec2(850, 650), skillColor, 1.5f);
    renderText(countText, glm::vec2(910, 600), usesColor, 3.0f);
    renderText(usesText, glm::vec2(890, 680), usesColor, 1.2f);
    
    end2DMode();
}

void UIRenderer::renderBurstJumpUI(bool hasSkill, bool isActive, int remainingUses, int maxUses) {
    // スキルを取得していない場合は表示しない
    if (!hasSkill) {
        return;
    }
    
    begin2DMode();
    
    // バーストジャンプスキルの表示（左下、フリーカメラスキルの上）
    std::string skillText = "BURST JUMP";
    glm::vec3 skillColor = glm::vec3(1.0f, 1.0f, 1.0f);
    
    // 使用回数表示
    std::string usesText = "PRESS B";
    glm::vec3 usesColor = glm::vec3(1.0f, 1.0f, 1.0f);
    
    // 使用回数が0の場合は灰色
    if (remainingUses <= 0) {
        usesColor = glm::vec3(0.5f, 0.5f, 0.5f);
        skillColor = glm::vec3(0.5f, 0.5f, 0.5f);
    }

    std::string countText = std::to_string(remainingUses);
    
    // アクティブ中の表示
    if (isActive) {
        std::string activeText = "PRESS SPACE!";
        renderText(activeText, glm::vec2(550, 490), glm::vec3(1.0f, 0.5f, 0.5f), 2.0f);
        usesColor = glm::vec3(1.0f, 0.5f, 0.5f);
        skillColor = glm::vec3(1.0f, 0.5f, 0.5f);
    }
    renderText(skillText, glm::vec2(650, 650), skillColor, 1.5f);

    renderText(countText, glm::vec2(700, 600), usesColor, 3.0f);
    
    renderText(usesText, glm::vec2(675, 680), usesColor, 1.2f);
    
    end2DMode();
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
