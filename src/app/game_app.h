#pragma once

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include <memory>
#include "../game/ecs.h"
#include "../game/systems.h"
#include "../gfx/vulkan_device.h"
#include "../core/time.h"

namespace app {

class GameApp {
public:
    GameApp();
    ~GameApp();

    bool initialize();
    void run();
    void cleanup();

private:
    bool initializeWindow();
    bool initializeVulkan();
    void setupInputCallbacks();
    void createGameEntities();
    
    void update(float deltaTime);
    void render();
    
    // GLFWコールバック
    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
    static void cursorPosCallback(GLFWwindow* window, double xpos, double ypos);
    static void windowResizeCallback(GLFWwindow* window, int width, int height);
    static void framebufferResizeCallback(GLFWwindow* window, int width, int height);

    // ウィンドウ
    GLFWwindow* window = nullptr;
    int windowWidth = 1920;
    int windowHeight = 1080;
    bool framebufferResized = false;
    
    // Vulkan
    VkInstance instance = VK_NULL_HANDLE;
    VkSurfaceKHR surface = VK_NULL_HANDLE;
    std::unique_ptr<gfx::VulkanDevice> device;
    
    // ゲームシステム
    std::unique_ptr<game::ECS> ecs;
    
    // 状態
    bool running = false;
    bool paused = false;
    
    // 時間
    core::Time time;
};

} // namespace app
