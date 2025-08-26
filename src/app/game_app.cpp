#include "game_app.h"
#include "../core/log.h"
#include "../core/math_utils.h"
#include "../game/components.h"
#include <stdexcept>

namespace app {

GameApp::GameApp() {
}

GameApp::~GameApp() {
    cleanup();
}

bool GameApp::initialize() {
    LOG_INFO("Initializing Vulkan3D application");
    
    if (!initializeWindow()) {
        LOG_ERROR("Failed to initialize window");
        return false;
    }
    
    if (!initializeVulkan()) {
        LOG_ERROR("Failed to initialize Vulkan");
        return false;
    }
    
    setupInputCallbacks();
    
    // ECSシステムの初期化
    ecs = std::make_unique<game::ECS>();
    
    // システムの登録
    ecs->addSystem("Input", game::inputSystem);
    ecs->addSystem("Player", game::playerSystem);
    ecs->addSystem("Physics", game::physicsSystem);
    ecs->addSystem("Portal", game::portalSystem);
    ecs->addSystem("Switch", game::switchSystem);
    ecs->addSystem("Door", game::doorSystem);
    ecs->addSystem("Camera", game::cameraSystem);
    ecs->addSystem("Render", game::renderSystem);
    
    createGameEntities();
    
    running = true;
    LOG_INFO("Application initialized successfully");
    return true;
}

void GameApp::run() {
    LOG_INFO("Starting game loop");
    
    while (running && !glfwWindowShouldClose(window)) {
        glfwPollEvents();
        
        time.update();
        float deltaTime = time.getDeltaTime();
        
        // 固定時間ステップで物理更新
        while (time.shouldRunFixedStep()) {
            if (!paused) {
                ecs->update(time.getFixedDeltaTime());
            }
            time.consumeFixedStep();
        }
        
        // 可変時間ステップでレンダリング
        if (!paused) {
            render();
        }
    }
    
    LOG_INFO("Game loop ended");
}

void GameApp::cleanup() {
    LOG_INFO("Cleaning up application");
    
    if (device) {
        device->cleanup();
    }
    
    if (surface != VK_NULL_HANDLE) {
        vkDestroySurfaceKHR(instance, surface, nullptr);
    }
    
    if (instance != VK_NULL_HANDLE) {
        vkDestroyInstance(instance, nullptr);
    }
    
    if (window) {
        glfwDestroyWindow(window);
    }
    
    glfwTerminate();
}

bool GameApp::initializeWindow() {
    if (!glfwInit()) {
        LOG_ERROR("Failed to initialize GLFW");
        return false;
    }
    
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    
    window = glfwCreateWindow(windowWidth, windowHeight, "Vulkan3D - Portal Physics Puzzle", nullptr, nullptr);
    if (!window) {
        LOG_ERROR("Failed to create GLFW window");
        glfwTerminate();
        return false;
    }
    
    glfwSetWindowUserPointer(window, this);
    
    return true;
}

bool GameApp::initializeVulkan() {
    // Vulkanインスタンスの作成
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Vulkan3D";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "Vulkan3D Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_3;
    
    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    
    // 拡張の設定
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    
    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
    
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();
    
    if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
        LOG_ERROR("Failed to create Vulkan instance");
        return false;
    }
    
    // サーフェスの作成
    if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
        LOG_ERROR("Failed to create window surface");
        return false;
    }
    
    // デバイスの初期化
    device = std::make_unique<gfx::VulkanDevice>();
    if (!device->initialize(instance, surface)) {
        LOG_ERROR("Failed to initialize Vulkan device");
        return false;
    }
    
    return true;
}

void GameApp::setupInputCallbacks() {
    glfwSetKeyCallback(window, keyCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetCursorPosCallback(window, cursorPosCallback);
    glfwSetWindowSizeCallback(window, windowResizeCallback);
    glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
    
    // マウスをロック
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    game::g_input.mouseLocked = true;
}

void GameApp::createGameEntities() {
    // プレイヤーの作成
    game::Entity player = ecs->createEntity();
    
    game::Transform playerTransform;
    playerTransform.position = core::Vec3(0, 1, -4);
    
    game::Player playerComp;
    playerComp.walkSpeed = 4.5f;
    playerComp.runSpeed = 6.0f;
    playerComp.jumpVelocity = 4.5f;
    playerComp.mouseSensitivity = 0.002f;
    
    game::Rigidbody playerRigidbody;
    playerRigidbody.mass = 70.0f;
    playerRigidbody.isKinematic = false;
    
    game::Collider playerCollider;
    playerCollider.shape = game::Collider::Shape::Capsule;
    playerCollider.radius = 0.3f;
    playerCollider.height = 1.8f;
    playerCollider.layer = 1; // Player layer
    
    game::Camera playerCamera;
    playerCamera.fov = 70.0f;
    playerCamera.aspectRatio = static_cast<float>(windowWidth) / windowHeight;
    
    ecs->addComponent(player, playerTransform);
    ecs->addComponent(player, playerComp);
    ecs->addComponent(player, playerRigidbody);
    ecs->addComponent(player, playerCollider);
    ecs->addComponent(player, playerCamera);
    
    // 床の作成
    game::Entity floor = ecs->createEntity();
    
    game::Transform floorTransform;
    floorTransform.position = core::Vec3(0, 0, 0);
    floorTransform.scale = core::Vec3(10, 1, 10);
    
    game::Renderable floorRenderable;
    floorRenderable.meshName = "cube";
    floorRenderable.materialName = "floor";
    
    game::Rigidbody floorRigidbody;
    floorRigidbody.isKinematic = true;
    
    game::Collider floorCollider;
    floorCollider.shape = game::Collider::Shape::Box;
    floorCollider.size = core::Vec3(10, 1, 10);
    floorCollider.layer = 2; // Static layer
    
    ecs->addComponent(floor, floorTransform);
    ecs->addComponent(floor, floorRenderable);
    ecs->addComponent(floor, floorRigidbody);
    ecs->addComponent(floor, floorCollider);
    
    // キューブの作成
    game::Entity cube = ecs->createEntity();
    
    game::Transform cubeTransform;
    cubeTransform.position = core::Vec3(-1, 1, 0);
    
    game::Renderable cubeRenderable;
    cubeRenderable.meshName = "cube";
    cubeRenderable.materialName = "metal";
    
    game::Rigidbody cubeRigidbody;
    cubeRigidbody.mass = 10.0f;
    
    game::Collider cubeCollider;
    cubeCollider.shape = game::Collider::Shape::Box;
    cubeCollider.size = core::Vec3(1, 1, 1);
    cubeCollider.layer = 4; // Dynamic layer
    
    game::Grabbable cubeGrabbable;
    
    ecs->addComponent(cube, cubeTransform);
    ecs->addComponent(cube, cubeRenderable);
    ecs->addComponent(cube, cubeRigidbody);
    ecs->addComponent(cube, cubeCollider);
    ecs->addComponent(cube, cubeGrabbable);
    
    // スイッチの作成
    game::Entity switchEntity = ecs->createEntity();
    
    game::Transform switchTransform;
    switchTransform.position = core::Vec3(1, 0.1f, 0);
    switchTransform.scale = core::Vec3(1, 0.2f, 1);
    
    game::Renderable switchRenderable;
    switchRenderable.meshName = "cube";
    switchRenderable.materialName = "switch";
    
    game::Rigidbody switchRigidbody;
    switchRigidbody.isKinematic = true;
    switchRigidbody.isTrigger = true;
    
    game::Collider switchCollider;
    switchCollider.shape = game::Collider::Shape::Box;
    switchCollider.size = core::Vec3(1, 0.2f, 1);
    switchCollider.layer = 8; // Trigger layer
    
    game::Switch switchComp;
    switchComp.thresholdMass = 10.0f;
    switchComp.debounceTime = 0.2f;
    
    ecs->addComponent(switchEntity, switchTransform);
    ecs->addComponent(switchEntity, switchRenderable);
    ecs->addComponent(switchEntity, switchRigidbody);
    ecs->addComponent(switchEntity, switchCollider);
    ecs->addComponent(switchEntity, switchComp);
    
    // ドアの作成
    game::Entity door = ecs->createEntity();
    
    game::Transform doorTransform;
    doorTransform.position = core::Vec3(5, 1, 0);
    doorTransform.scale = core::Vec3(1, 2, 0.2f);
    
    game::Renderable doorRenderable;
    doorRenderable.meshName = "cube";
    doorRenderable.materialName = "door";
    
    game::Rigidbody doorRigidbody;
    doorRigidbody.isKinematic = true;
    
    game::Collider doorCollider;
    doorCollider.shape = game::Collider::Shape::Box;
    doorCollider.size = core::Vec3(1, 2, 0.2f);
    doorCollider.layer = 2; // Static layer
    
    game::Door doorComp;
    doorComp.logic = game::Door::Logic::AND;
    doorComp.inputSwitches.push_back(switchEntity);
    doorComp.closedPosition = core::Vec3(5, 1, 0);
    doorComp.openPosition = core::Vec3(5, 3, 0);
    
    ecs->addComponent(door, doorTransform);
    ecs->addComponent(door, doorRenderable);
    ecs->addComponent(door, doorRigidbody);
    ecs->addComponent(door, doorCollider);
    ecs->addComponent(door, doorComp);
    
    LOG_INFO("Created {} game entities", ecs->query<game::Transform>().size());
}

void GameApp::update(float deltaTime) {
    // ECSシステムの更新はrun()で行われる
}

void GameApp::render() {
    // TODO: Vulkanレンダラーの実装
    // 現在はプレースホルダー
}

// GLFWコールバック関数
void GameApp::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    auto app = static_cast<GameApp*>(glfwGetWindowUserPointer(window));
    
    if (action == GLFW_PRESS) {
        game::g_input.keys[key] = true;
        
        if (key == GLFW_KEY_ESCAPE) {
            app->running = false;
        }
        
        if (key == GLFW_KEY_P) {
            app->paused = !app->paused;
            LOG_INFO("Game {}", app->paused ? "paused" : "resumed");
        }
    } else if (action == GLFW_RELEASE) {
        game::g_input.keys[key] = false;
    }
}

void GameApp::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    if (action == GLFW_PRESS) {
        game::g_input.mouseButtons[button] = true;
    } else if (action == GLFW_RELEASE) {
        game::g_input.mouseButtons[button] = false;
    }
}

void GameApp::cursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
    static double lastX = xpos;
    static double lastY = ypos;
    
    if (game::g_input.mouseLocked) {
        game::g_input.mouseDelta.x = static_cast<float>(xpos - lastX);
        game::g_input.mouseDelta.y = static_cast<float>(ypos - lastY);
    }
    
    lastX = xpos;
    lastY = ypos;
}

void GameApp::windowResizeCallback(GLFWwindow* window, int width, int height) {
    auto app = static_cast<GameApp*>(glfwGetWindowUserPointer(window));
    app->windowWidth = width;
    app->windowHeight = height;
    LOG_INFO("Window resized to {}x{}", width, height);
}

void GameApp::framebufferResizeCallback(GLFWwindow* window, int width, int height) {
    auto app = static_cast<GameApp*>(glfwGetWindowUserPointer(window));
    app->framebufferResized = true;
    LOG_INFO("Framebuffer resized to {}x{}", width, height);
}

} // namespace app
