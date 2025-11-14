#pragma once

#include <vector>
#include <glm/glm.hpp>

/**
 * @brief アイテムの状態を管理する構造体
 */
struct Item {
    glm::vec3 position;
    glm::vec3 size;
    glm::vec3 color;
    bool isCollected;
    int itemId;
    float rotationAngle;  // 回転角度（単位: 度）
    float bobHeight;  // 上下動の高さ（単位: ワールド座標）
    float bobTimer;  // 上下動タイマー（単位: 秒）
};

/**
 * @brief アイテムシステムの状態を管理する構造体
 */
struct ItemState {
    std::vector<Item> items;
    int collectedItems = 0;
    int requiredItems = 3;
    int earnedItems = 0;
    int totalItems = 3;
};

