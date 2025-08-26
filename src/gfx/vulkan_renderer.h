#pragma once

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <memory>
#include <string>
#include <glm/glm.hpp>
#include "vulkan_device.h"

namespace gfx {

class VulkanRenderer {
public:
    VulkanRenderer();
    ~VulkanRenderer();

    bool initialize(GLFWwindow* window);
    void cleanup();
    
    void beginFrame();
    void endFrame();
    
    void renderTriangle(const glm::vec3& position, const glm::vec3& color);
    void renderCube(const glm::vec3& position, const glm::vec3& color);
    void renderText(const std::string& text, const glm::vec2& position, const glm::vec3& color);

private:
    bool createInstance();
    bool createSurface();
    bool createSwapChain();
    bool createImageViews();
    bool createRenderPass();
    bool createFramebuffers();
    bool createCommandPool();
    bool createCommandBuffers();
    bool createSyncObjects();
    
    void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
    void drawFrame();

    GLFWwindow* window;
    VkInstance instance;
    VkSurfaceKHR surface;
    std::unique_ptr<VulkanDevice> device;
    
    VkSwapchainKHR swapChain;
    std::vector<VkImage> swapChainImages;
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;
    std::vector<VkImageView> swapChainImageViews;
    
    VkRenderPass renderPass;
    std::vector<VkFramebuffer> swapChainFramebuffers;
    
    VkCommandPool commandPool;
    std::vector<VkCommandBuffer> commandBuffers;
    
    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;
    size_t currentFrame = 0;
    
    bool framebufferResized = false;
};

} // namespace gfx
