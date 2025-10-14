#pragma once

// キー入力の状態管理
struct KeyState {
    bool isPressed = false;
    bool wasPressed = false;
    
    void update(bool currentlyPressed) {
        wasPressed = isPressed;
        isPressed = currentlyPressed;
    }
    
    bool justPressed() const { return isPressed && !wasPressed; }
    bool justReleased() const { return !isPressed && wasPressed; }
};
