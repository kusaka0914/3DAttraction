#pragma once
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <string>
#include <map>
#include <set>
#include "../core/constants/game_constants.h"
#include "bitmap_font.h"

namespace gfx {

/**
 * @brief UI状態を管理する構造体
 * @details ゲームUIの表示状態を保持します。
 */
struct GameUIState {
    bool showTime = true;
    bool showGoal = true;
    bool showStars = true;
    bool showLives = true;
    
    float remainingTime = 0.0f;
    float timeLimit = 0.0f;
    int existingStars = 0;
    int lives = 6;
    
    int currentStage = -1;
    int selectedSecretStarType = -1;
    std::map<int, std::set<int>> secretStarCleared;
};

/**
 * @brief スキルUI設定を管理する構造体
 * @details スキルUIの表示設定を保持します。
 */
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

/**
 * @brief UIレンダラー
 * @details ゲームUIの描画を統合的に管理します。
 */
class UIRenderer {
public:
    UIRenderer();
    ~UIRenderer();
    
    /**
     * @brief ウィンドウサイズを設定する
     * @details スケーリング用にウィンドウサイズを設定します。
     * 
     * @param width ウィンドウ幅
     * @param height ウィンドウ高さ
     */
    void setWindowSize(int width, int height);
    
    /**
     * @brief 統合されたUI描画関数
     * @details ゲームUIを統合的に描画します。
     * 
     * @param state UI状態
     */
    void renderGameUI(const GameUIState& state);
    
    void renderTimeUI(float remainingTime, float timeLimit, int earnedStars, int existingStars, int lives, 
                      int currentStage = -1, int selectedSecretStarType = -1, const std::map<int, std::set<int>>& secretStarCleared = {});
    void renderTimeAttackUI(float currentTime, float bestTime, int earnedStars, int existingStars, int lives, float timeScale, bool isReplayMode = false);
    void renderLivesWithExplanation(int lives);
    void renderLivesAndTimeUI(int lives, float remainingTime, float timeLimit, int earnedStars, int existingStars);
    void renderLivesTimeAndStarsUI(int lives, float remainingTime, float timeLimit, int earnedStars, int existingStars,
                                   int currentStage = -1, int selectedSecretStarType = -1, const std::map<int, std::set<int>>& secretStarCleared = {});
    
    /**
     * @brief 統合されたスキルUI描画関数
     * @details スキルUIを統合的に描画します。
     * 
     * @param config スキルUI設定
     * @param hasSkill スキルを持っているか
     * @param isActive スキルがアクティブか
     * @param remainingUses 残り使用回数
     * @param maxUses 最大使用回数
     * @param isEasyMode EASYモードか
     */
    void renderSkillUI(const SkillUIConfig& config, bool hasSkill, bool isActive, 
                       int remainingUses, int maxUses, bool isEasyMode = false);
    
    void renderTimeStopUI(bool hasSkill, bool isTimeStopped, float timeStopTimer, int remainingUses, int maxUses);
    void renderDoubleJumpUI(bool hasSkill, bool isEasyMode, int remainingUses, int maxUses);
    void renderHeartFeelUI(bool hasSkill, int remainingUses, int maxUses, int currentLives);
    void renderFreeCameraUI(bool hasSkill, bool isActive, float timer, int remainingUses, int maxUses);
    void renderBurstJumpUI(bool hasSkill, bool isActive, int remainingUses, int maxUses);
    
    /**
     * @brief 星を描画する
     * @param position 描画位置
     * @param color 色
     * @param scale スケール
     */
    void renderStar(const glm::vec2& position, const glm::vec3& color, float scale);
    
    /**
     * @brief ハートを描画する
     * @param position 描画位置
     * @param color 色
     * @param scale スケール
     */
    void renderHeart(const glm::vec2& position, const glm::vec3& color, float scale);
    
    /**
     * @brief テキストを描画する
     * @param text 描画するテキスト
     * @param position 描画位置
     * @param color 色
     * @param scale スケール
     */
    void renderText(const std::string& text, const glm::vec2& position, const glm::vec3& color, float scale = 1.0f);
    
    /**
     * @brief 2D描画モードを開始する
     * @details 2D描画用の設定を行います。
     */
    void begin2DMode();
    
    /**
     * @brief 2D描画モードを終了する
     * @details 2D描画用の設定を解除します。
     */
    void end2DMode();

private:
    void renderTimeDisplay(float remainingTime, float timeLimit);
    void renderTimeAttackDisplay(float currentTime, float bestTime);
    void renderGoalDisplay(float timeLimit);
    void renderStarsDisplay(int existingStars, int currentStage = -1, int selectedSecretStarType = -1, const std::map<int, std::set<int>>& secretStarCleared = {});
    void renderLivesDisplay(int lives);
    
    void renderExplanationText(const std::string& type, const glm::vec2& position);
    
    void renderBitmapChar(char c, const glm::vec2& position, const glm::vec3& color, float scale);
    
    BitmapFont font;
    
    int windowWidth = 1280;
    int windowHeight = 720;
};

} // namespace gfx
