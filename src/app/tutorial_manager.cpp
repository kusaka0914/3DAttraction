#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "tutorial_manager.h"

namespace TutorialManager {
    void processTutorialProgress(GLFWwindow* window, GameState& gameState, 
                                const std::map<int, InputUtils::KeyState>& keyStates) {
        if (gameState.progress.tutorialStepCompleted && keyStates.at(GLFW_KEY_ENTER).justPressed()) {
            gameState.progress.tutorialStep++;
            gameState.progress.tutorialStepCompleted = false;
            gameState.progress.tutorialStartPosition = gameState.player.position;
            
            switch (gameState.progress.tutorialStep) {
                case 1: gameState.ui.tutorialMessage = "MOVING LEFT: PRESS A"; break;
                case 2: gameState.ui.tutorialMessage = "MOVING BACKWARD: PRESS S"; break;
                case 3: gameState.ui.tutorialMessage = "MOVING RIGHT: PRESS D"; break;
                case 4: gameState.ui.tutorialMessage = "JUMPING: PRESS SPACE"; break;
                case 5: gameState.ui.tutorialMessage = "SPEED UP: PRESS T"; break;
                case 6: gameState.ui.tutorialMessage = "LIFE"; break;
                case 7: gameState.ui.tutorialMessage = "TIME LIMIT"; break;
                case 8: gameState.ui.tutorialMessage = "STAR"; break;
                case 9: gameState.ui.tutorialMessage = "ITEM"; break;
                case 10: gameState.ui.tutorialMessage = "GOAL"; break;
                default: gameState.ui.tutorialMessage = ""; break;
            }
        }
        
        if (!gameState.progress.tutorialStepCompleted) {
            bool stepCompleted = false;
            
            if (gameState.progress.tutorialStep >= 6 && gameState.progress.tutorialStep < 9) {
                stepCompleted = true;
            } else if (gameState.progress.tutorialStep == 4) {
                stepCompleted = (gameState.player.velocity.y > 0.0f);
            } else if (gameState.progress.tutorialStep == 5) {
                stepCompleted = (gameState.progress.timeScale != 1.0f);
            } else if (gameState.progress.tutorialStep == 9) {
                stepCompleted = (gameState.items.earnedItems >= gameState.items.totalItems);
            } else if (gameState.progress.tutorialStep == 10) {
                stepCompleted = gameState.progress.isGoalReached;
            } else {
                float distance = glm::length(gameState.player.position - gameState.progress.tutorialStartPosition);
                stepCompleted = (distance >= gameState.progress.tutorialRequiredDistance);
            }
            
            if (stepCompleted) {
                gameState.progress.tutorialStepCompleted = true;
            }
        }
    }
    
    bool isTutorialInputEnabled(const GameState& gameState, GLFWwindow* window, 
                               const std::map<int, InputUtils::KeyState>& keyStates) {
        if (!gameState.progress.isTutorialStage) {
            return true;
        }
        
        if (gameState.progress.tutorialStepCompleted) {
            return false;  // ステップ完了時は入力無効
        } else if (gameState.progress.tutorialStep == 6) {
            return false;  // ステップ6は動けない
        } else if (gameState.progress.tutorialStep >= 7) {
            return true;   // ステップ7以降は動ける（アイテム取得やゴール到達のため）
        } else {
            switch (gameState.progress.tutorialStep) {
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



