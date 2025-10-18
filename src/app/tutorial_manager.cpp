#include "tutorial_manager.h"

namespace TutorialManager {
    void processTutorialProgress(GLFWwindow* window, GameState& gameState, 
                                const std::map<int, InputUtils::KeyState>& keyStates) {
        if (gameState.tutorialStepCompleted && keyStates.at(GLFW_KEY_ENTER).justPressed()) {
            // 次のステップに進む
            gameState.tutorialStep++;
            gameState.tutorialStepCompleted = false;
            gameState.tutorialStartPosition = gameState.playerPosition;
            
            // 次のステップのメッセージを設定
            switch (gameState.tutorialStep) {
                case 1: gameState.tutorialMessage = "MOVING LEFT: PRESS A"; break;
                case 2: gameState.tutorialMessage = "MOVING BACKWARD: PRESS S"; break;
                case 3: gameState.tutorialMessage = "MOVING RIGHT: PRESS D"; break;
                case 4: gameState.tutorialMessage = "JUMPING: PRESS SPACE"; break;
                case 5: gameState.tutorialMessage = "SPEED UP: PRESS T"; break;
                case 6: gameState.tutorialMessage = "LIFE"; break;
                case 7: gameState.tutorialMessage = "TIME LIMIT"; break;
                case 8: gameState.tutorialMessage = "STAR"; break;
                case 9: gameState.tutorialMessage = "ITEM"; break;
                case 10: gameState.tutorialMessage = "GOAL"; break;
                default: gameState.tutorialMessage = ""; break;
            }
        }
        
        // 現在のステップでの完了条件をチェック
        if (!gameState.tutorialStepCompleted) {
            bool stepCompleted = false;
            
            if (gameState.tutorialStep >= 6 && gameState.tutorialStep < 9) {
                // STEP 6以降は即座に完了（Enterで進むため）
                stepCompleted = true;
            } else if (gameState.tutorialStep == 4) {
                // SPACEキーのステップ：ジャンプが実行されたかチェック（velocity.yが正の値になったかどうか）
                stepCompleted = (gameState.playerVelocity.y > 0.0f);
            } else if (gameState.tutorialStep == 5) {
                // Tキーのステップ：速度変更が行われたかチェック
                stepCompleted = (gameState.timeScale != 1.0f);
            } else if (gameState.tutorialStep == 9) {
                // ステップ9：アイテムを3つ全て取った時に完了
                stepCompleted = (gameState.earnedItems >= gameState.totalItems);
            } else if (gameState.tutorialStep == 10) {
                // ステップ10：ゴール地点に乗った時に完了
                stepCompleted = gameState.isGoalReached;
            } else {
                // その他のステップ：移動距離をチェック
                float distance = glm::length(gameState.playerPosition - gameState.tutorialStartPosition);
                stepCompleted = (distance >= gameState.tutorialRequiredDistance);
            }
            
            if (stepCompleted) {
                gameState.tutorialStepCompleted = true;
            }
        }
    }
    
    bool isTutorialInputEnabled(const GameState& gameState, GLFWwindow* window, 
                               const std::map<int, InputUtils::KeyState>& keyStates) {
        if (!gameState.isTutorialStage) {
            return true;
        }
        
        if (gameState.tutorialStepCompleted) {
            return false;  // ステップ完了時は入力無効
        } else if (gameState.tutorialStep == 6) {
            return false;  // ステップ6は動けない
        } else if (gameState.tutorialStep >= 7) {
            return true;   // ステップ7以降は動ける（アイテム取得やゴール到達のため）
        } else {
            // 現在のステップ以外のキーを無効化
            switch (gameState.tutorialStep) {
                case 0: // Wキーのステップ
                    return (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS);
                case 1: // Aキーのステップ
                    return (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS);
                case 2: // Sキーのステップ
                    return (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS);
                case 3: // Dキーのステップ
                    return (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS);
                case 4: // SPACEキーのステップ
                    return (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS);
                case 5: // Tキーのステップ
                    return keyStates.at(GLFW_KEY_T).justPressed();
                default:
                    return true; // ステップ6以降は全て有効
            }
        }
    }
}

