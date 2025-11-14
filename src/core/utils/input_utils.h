/**
 * @file input_utils.h
 * @brief 入力ユーティリティ
 * @details キー入力の状態管理を提供します。
 */
#pragma once

/**
 * @brief 入力ユーティリティ
 * @details キー入力の状態管理を提供します。
 */
namespace InputUtils {
    /**
     * @brief キー入力の状態を管理する構造体
     * @details キーの押下状態と前フレームの状態を保持し、押下瞬間の検出を可能にします。
     */
    struct KeyState {
        bool isPressed = false;
        bool wasPressed = false;
        
        /**
         * @brief キー状態を更新する
         * @param currentlyPressed 現在の押下状態
         */
        void update(bool currentlyPressed) {
            wasPressed = isPressed;
            isPressed = currentlyPressed;
        }
        
        /**
         * @brief キーが今フレームで押されたか確認する
         * @return 今フレームで押された場合true
         */
        bool justPressed() const { return isPressed && !wasPressed; }
        
        /**
         * @brief キーが今フレームで離されたか確認する
         * @return 今フレームで離された場合true
         */
        bool justReleased() const { return !isPressed && wasPressed; }
    };
}
