/**
 * @file game_state_ui_renderer.h
 * @brief ゲーム状態UIレンダラー
 * @details ゲーム状態に応じたUI要素（タイトル画面、ステージクリア、チュートリアルなど）の描画を統合的に管理します。
 */
#pragma once
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <string>
#include "../core/constants/game_constants.h"
#include "bitmap_font.h"
#include "../game/game_state.h"
#include "../game/game_progress_state.h"

struct GameState;

namespace gfx {

/**
 * @brief ゲーム状態UIレンダラー
 * @details ゲーム状態に応じたUI要素の描画を統合的に管理します。
 */
class GameStateUIRenderer {
public:
    GameStateUIRenderer();
    ~GameStateUIRenderer();
    
    /**
     * @brief チュートリアルステージUIを描画する
     * @param width ウィンドウ幅
     * @param height ウィンドウ高さ
     * @param message メッセージ
     * @param currentStep 現在のステップ
     * @param stepCompleted ステップ完了フラグ
     */
    void renderTutorialStageUI(int width, int height, const std::string& message, int currentStep, bool stepCompleted);
    
    /**
     * @brief ステージクリア背景を描画する
     * @param width ウィンドウ幅
     * @param height ウィンドウ高さ
     * @param clearTime クリア時間
     * @param earnedStars 獲得星数
     * @param isTimeAttackMode タイムアタックモードか
     * @param currentStage 現在のステージ番号
     * @param selectedSecretStarType 選択されたSECRET STARタイプ
     * @param secretStarCleared クリア済みSECRET STARのマップ
     */
    void renderStageClearBackground(int width, int height, float clearTime, int earnedStars, bool isTimeAttackMode = false,
                                     int currentStage = -1, GameProgressState::SecretStarType selectedSecretStarType = GameProgressState::SecretStarType::NONE,
                                     const std::map<int, std::set<GameProgressState::SecretStarType>>& secretStarCleared = {});
    
    /**
     * @brief タイムアタッククリア背景を描画する
     * @param width ウィンドウ幅
     * @param height ウィンドウ高さ
     * @param clearTime クリア時間
     * @param bestTime ベストタイム
     * @param isNewRecord 新記録フラグ
     */
    void renderTimeAttackClearBackground(int width, int height, float clearTime, float bestTime, bool isNewRecord);
    
    /**
     * @brief ステージ解放確認背景を描画する
     * @param width ウィンドウ幅
     * @param height ウィンドウ高さ
     * @param targetStage 対象ステージ番号
     * @param requiredStars 必要な星数
     * @param currentStars 現在の星数
     */
    void renderUnlockConfirmBackground(int width, int height, int targetStage, int requiredStars, int currentStars);
    
    /**
     * @brief 星不足警告背景を描画する
     * @param width ウィンドウ幅
     * @param height ウィンドウ高さ
     * @param targetStage 対象ステージ番号
     * @param requiredStars 必要な星数
     * @param currentStars 現在の星数
     */
    void renderStarInsufficientBackground(int width, int height, int targetStage, int requiredStars, int currentStars);
    
    /**
     * @brief ワープチュートリアル背景を描画する
     * @param width ウィンドウ幅
     * @param height ウィンドウ高さ
     * @param targetStage 対象ステージ番号
     */
    void renderWarpTutorialBackground(int width, int height, int targetStage);
    
    /**
     * @brief ゲームオーバー背景を描画する
     * @param width ウィンドウ幅
     * @param height ウィンドウ高さ
     */
    void renderGameOverBackground(int width, int height);
    
    /**
     * @brief タイトル画面を描画する
     * @param width ウィンドウ幅
     * @param height ウィンドウ高さ
     * @param gameState ゲーム状態
     */
    void renderTitleScreen(int width, int height, const GameState& gameState);
    
    /**
     * @brief Ready画面を描画する
     * @param width ウィンドウ幅
     * @param height ウィンドウ高さ
     * @param speedLevel 速度レベル
     * @param isFirstPersonMode 1人称モードか
     */
    void renderReadyScreen(int width, int height, int speedLevel, bool isFirstPersonMode);
    
    /**
     * @brief カウントダウンを描画する
     * @param width ウィンドウ幅
     * @param height ウィンドウ高さ
     * @param count カウントダウン数値
     */
    void renderCountdown(int width, int height, int count);
    
    /**
     * @brief ステージ0チュートリアルを描画する
     * @param width ウィンドウ幅
     * @param height ウィンドウ高さ
     */
    void renderStage0Tutorial(int width, int height);
    
    /**
     * @brief EASYモード説明UIを描画する
     * @param width ウィンドウ幅
     * @param height ウィンドウ高さ
     */
    void renderEasyModeExplanationUI(int width, int height);
    
    /**
     * @brief EASYモード選択UIを描画する
     * @param width ウィンドウ幅
     * @param height ウィンドウ高さ
     * @param isEasyMode EASYモードか
     */
    void renderEasyModeSelectionUI(int width, int height, bool isEasyMode);
    
    /**
     * @brief タイムアタック選択UIを描画する
     * @param width ウィンドウ幅
     * @param height ウィンドウ高さ
     * @param isTimeAttackMode タイムアタックモードか
     * @param isGameCleared ゲームクリア済みか
     * @param secretStarType SECRET STARタイプ
     */
    void renderTimeAttackSelectionUI(int width, int height, bool isTimeAttackMode, bool isGameCleared, GameProgressState::SecretStarType secretStarType);
    
    /**
     * @brief SECRET STAR説明UIを描画する
     * @param width ウィンドウ幅
     * @param height ウィンドウ高さ
     */
    void renderSecretStarExplanationUI(int width, int height);
    
    /**
     * @brief SECRET STAR選択UIを描画する
     * @param width ウィンドウ幅
     * @param height ウィンドウ高さ
     * @param selectedType 選択されたタイプ
     */
    void renderSecretStarSelectionUI(int width, int height, GameProgressState::SecretStarType selectedType);
    
    /**
     * @brief ステージ選択アシストを描画する
     * @param width ウィンドウ幅
     * @param height ウィンドウ高さ
     * @param targetStage 対象ステージ番号
     * @param isVisible 表示フラグ
     * @param isUnlocked 解放済みフラグ
     */
    void renderStageSelectionAssist(int width, int height, int targetStage, bool isVisible, bool isUnlocked);
    
    /**
     * @brief ステージ選択画面のESCキー情報を描画する
     * @param width ウィンドウ幅
     * @param height ウィンドウ高さ
     */
    void renderStageSelectionEscKeyInfo(int width, int height);
    
    /**
     * @brief ランキングボードアシストを描画する
     * @param width ウィンドウ幅
     * @param height ウィンドウ高さ
     * @param isVisible 表示フラグ
     */
    void renderLeaderboardAssist(int width, int height, bool isVisible);
    
    /**
     * @brief ランキングUIを描画する
     * @param width ウィンドウ幅
     * @param height ウィンドウ高さ
     * @param targetStage 対象ステージ番号
     * @param entries ランキングエントリ
     * @param isLoading 読み込み中フラグ
     */
    void renderLeaderboardUI(int width, int height, int targetStage, 
                               const std::vector<LeaderboardEntry>& entries, 
                               bool isLoading, int selectedIndex = 0);
    
    /**
     * @brief プレイヤー名入力画面を描画する
     * @param width ウィンドウ幅
     * @param height ウィンドウ高さ
     * @param playerName 入力中のプレイヤー名
     * @param cursorPos カーソル位置
     * @param timer タイマー（カーソル点滅用）
     */
    void renderPlayerNameInput(int width, int height, const std::string& playerName, int cursorPos, float timer = 0.0f);
    
    /**
     * @brief エンディングメッセージを描画する
     * @param width ウィンドウ幅
     * @param height ウィンドウ高さ
     * @param timer タイマー
     */
    void renderEndingMessage(int width, int height, float timer);
    
    /**
     * @brief スタッフロールを描画する
     * @param width ウィンドウ幅
     * @param height ウィンドウ高さ
     * @param timer タイマー
     */
    void renderStaffRoll(int width, int height, float timer);
    
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
    void renderText(const std::string& text, const glm::vec2& position, const glm::vec3& color, float scale = 1.0f);
    void renderBitmapChar(char c, const glm::vec2& position, const glm::vec3& color, float scale);
    void renderStar(const glm::vec2& position, const glm::vec3& color, float scale, int width, int height);
    
    BitmapFont font;
};
} // namespace gfx
