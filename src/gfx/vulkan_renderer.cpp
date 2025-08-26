#include "vulkan_renderer.h"
#include <stdexcept>
#include <iostream>
#include <algorithm>
#include <set>
#include <vector>
#include <vulkan/vulkan_metal.h>

#ifndef VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME
#define VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME "VK_KHR_portability_enumeration"
#endif
#ifndef VK_EXT_METAL_SURFACE_EXTENSION_NAME
#define VK_EXT_METAL_SURFACE_EXTENSION_NAME "VK_EXT_metal_surface"
#endif
#ifndef VK_KHR_SURFACE_EXTENSION_NAME
#define VK_KHR_SURFACE_EXTENSION_NAME "VK_KHR_surface"
#endif

namespace gfx {

VulkanRenderer::VulkanRenderer() : window(nullptr), instance(VK_NULL_HANDLE), surface(VK_NULL_HANDLE), swapChain(VK_NULL_HANDLE) {
}

VulkanRenderer::~VulkanRenderer() {
    cleanup();
}

bool VulkanRenderer::initialize(GLFWwindow* window) {
    this->window = window;
    
    std::cout << "Creating Vulkan instance..." << std::endl;
    if (!createInstance()) {
        std::cerr << "Failed to create Vulkan instance" << std::endl;
        return false;
    }
    
    std::cout << "Creating surface..." << std::endl;
    if (!createSurface()) {
        std::cerr << "Failed to create surface" << std::endl;
        return false;
    }
    
    std::cout << "Initializing Vulkan device..." << std::endl;
    device = std::make_unique<VulkanDevice>();
    if (!device->initialize(instance, surface)) {
        std::cerr << "Failed to initialize Vulkan device" << std::endl;
        return false;
    }
    
    std::cout << "Creating swap chain..." << std::endl;
    if (!createSwapChain()) {
        std::cerr << "Failed to create swap chain" << std::endl;
        return false;
    }
    
    std::cout << "Creating image views..." << std::endl;
    if (!createImageViews()) {
        std::cerr << "Failed to create image views" << std::endl;
        return false;
    }
    
    std::cout << "Creating render pass..." << std::endl;
    if (!createRenderPass()) {
        std::cerr << "Failed to create render pass" << std::endl;
        return false;
    }
    
    std::cout << "Creating framebuffers..." << std::endl;
    if (!createFramebuffers()) {
        std::cerr << "Failed to create framebuffers" << std::endl;
        return false;
    }
    
    std::cout << "Creating command pool..." << std::endl;
    if (!createCommandPool()) {
        std::cerr << "Failed to create command pool" << std::endl;
        return false;
    }
    
    std::cout << "Creating command buffers..." << std::endl;
    if (!createCommandBuffers()) {
        std::cerr << "Failed to create command buffers" << std::endl;
        return false;
    }
    
    std::cout << "Creating sync objects..." << std::endl;
    if (!createSyncObjects()) {
        std::cerr << "Failed to create sync objects" << std::endl;
        return false;
    }
    
    std::cout << "Vulkan renderer initialized successfully" << std::endl;
    return true;
}

void VulkanRenderer::cleanup() {
    if (device) {
        vkDeviceWaitIdle(device->getDevice());
        
        for (auto fence : inFlightFences) {
            vkDestroyFence(device->getDevice(), fence, nullptr);
        }
        
        for (auto semaphore : renderFinishedSemaphores) {
            vkDestroySemaphore(device->getDevice(), semaphore, nullptr);
        }
        
        for (auto semaphore : imageAvailableSemaphores) {
            vkDestroySemaphore(device->getDevice(), semaphore, nullptr);
        }
        
        vkDestroyCommandPool(device->getDevice(), commandPool, nullptr);
        
        for (auto framebuffer : swapChainFramebuffers) {
            vkDestroyFramebuffer(device->getDevice(), framebuffer, nullptr);
        }
        
        vkDestroyRenderPass(device->getDevice(), renderPass, nullptr);
        
        for (auto imageView : swapChainImageViews) {
            vkDestroyImageView(device->getDevice(), imageView, nullptr);
        }
        
        vkDestroySwapchainKHR(device->getDevice(), swapChain, nullptr);
    }
    
    if (surface != VK_NULL_HANDLE) {
        vkDestroySurfaceKHR(instance, surface, nullptr);
    }
    
    if (instance != VK_NULL_HANDLE) {
        vkDestroyInstance(instance, nullptr);
    }
}

bool VulkanRenderer::createInstance() {
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Vulkan3D";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::cout << "Required GLFW extensions: " << glfwExtensionCount << std::endl;
    for (uint32_t i = 0; i < glfwExtensionCount; i++) {
        std::cout << "  " << glfwExtensions[i] << std::endl;
    }

    // デバッグ: 利用可能なインスタンス拡張を列挙
    uint32_t availCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &availCount, nullptr);
    std::vector<VkExtensionProperties> avail(availCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &availCount, avail.data());
    std::cout << "Available instance extensions (" << availCount << "):" << std::endl;
    for (const auto& e : avail) {
        std::cout << "  " << e.extensionName << std::endl;
    }

    std::vector<const char*> extensions;
    if (glfwExtensionCount > 0 && glfwExtensions != nullptr) {
        extensions.insert(extensions.end(), glfwExtensions, glfwExtensions + glfwExtensionCount);
    } else {
        // macOS + MoltenVK のフォールバック
        extensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
        extensions.push_back(VK_EXT_METAL_SURFACE_EXTENSION_NAME);
    }
    // ポータビリティ (MoltenVK) 推奨拡張
    extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);

    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();
    createInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;

    VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);
    if (result != VK_SUCCESS) {
        std::cerr << "Failed to create Vulkan instance. Error code: " << result << std::endl;
        return false;
    }

    std::cout << "Vulkan instance created successfully" << std::endl;
    return true;
}

bool VulkanRenderer::createSurface() {
    VkResult result = glfwCreateWindowSurface(instance, window, nullptr, &surface);
    if (result != VK_SUCCESS) {
        std::cerr << "Failed to create window surface. Error code: " << result << std::endl;
        return false;
    }
    std::cout << "Surface created successfully" << std::endl;
    return true;
}

bool VulkanRenderer::createSwapChain() {
    auto swapChainSupport = device->querySwapChainSupport(surface);

    VkSurfaceFormatKHR surfaceFormat = swapChainSupport.formats[0];
    for (const auto& availableFormat : swapChainSupport.formats) {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            surfaceFormat = availableFormat;
            break;
        }
    }

    VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
    for (const auto& availablePresentMode : swapChainSupport.presentModes) {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            presentMode = availablePresentMode;
            break;
        }
    }

    VkExtent2D extent = swapChainSupport.capabilities.currentExtent;
    if (extent.width == std::numeric_limits<uint32_t>::max()) {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        extent.width = static_cast<uint32_t>(width);
        extent.height = static_cast<uint32_t>(height);

        extent.width = std::clamp(extent.width, swapChainSupport.capabilities.minImageExtent.width, swapChainSupport.capabilities.maxImageExtent.width);
        extent.height = std::clamp(extent.height, swapChainSupport.capabilities.minImageExtent.height, swapChainSupport.capabilities.maxImageExtent.height);
    }

    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(device->getDevice(), &createInfo, nullptr, &swapChain) != VK_SUCCESS) {
        std::cerr << "Failed to create swap chain" << std::endl;
        return false;
    }

    vkGetSwapchainImagesKHR(device->getDevice(), swapChain, &imageCount, nullptr);
    swapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(device->getDevice(), swapChain, &imageCount, swapChainImages.data());

    swapChainImageFormat = surfaceFormat.format;
    swapChainExtent = extent;

    return true;
}

bool VulkanRenderer::createImageViews() {
    swapChainImageViews.resize(swapChainImages.size());

    for (size_t i = 0; i < swapChainImages.size(); i++) {
        VkImageViewCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = swapChainImages[i];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = swapChainImageFormat;
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(device->getDevice(), &createInfo, nullptr, &swapChainImageViews[i]) != VK_SUCCESS) {
            std::cerr << "Failed to create image views" << std::endl;
            return false;
        }
    }

    return true;
}

bool VulkanRenderer::createRenderPass() {
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = swapChainImageFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;

    if (vkCreateRenderPass(device->getDevice(), &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
        std::cerr << "Failed to create render pass" << std::endl;
        return false;
    }

    return true;
}

bool VulkanRenderer::createFramebuffers() {
    swapChainFramebuffers.resize(swapChainImageViews.size());

    for (size_t i = 0; i < swapChainImageViews.size(); i++) {
        VkImageView attachments[] = {
            swapChainImageViews[i]
        };

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = swapChainExtent.width;
        framebufferInfo.height = swapChainExtent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(device->getDevice(), &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS) {
            std::cerr << "Failed to create framebuffer" << std::endl;
            return false;
        }
    }

    return true;
}

bool VulkanRenderer::createCommandPool() {
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = device->getGraphicsQueueFamily();

    if (vkCreateCommandPool(device->getDevice(), &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
        std::cerr << "Failed to create command pool" << std::endl;
        return false;
    }

    return true;
}

bool VulkanRenderer::createCommandBuffers() {
    commandBuffers.resize(swapChainFramebuffers.size());

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

    if (vkAllocateCommandBuffers(device->getDevice(), &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
        std::cerr << "Failed to allocate command buffers" << std::endl;
        return false;
    }

    return true;
}

bool VulkanRenderer::createSyncObjects() {
    imageAvailableSemaphores.resize(2);
    renderFinishedSemaphores.resize(2);
    inFlightFences.resize(2);

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < 2; i++) {
        if (vkCreateSemaphore(device->getDevice(), &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(device->getDevice(), &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(device->getDevice(), &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {
            std::cerr << "Failed to create synchronization objects" << std::endl;
            return false;
        }
    }

    return true;
}

void VulkanRenderer::beginFrame() {
    vkWaitForFences(device->getDevice(), 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
}

void VulkanRenderer::endFrame() {
    drawFrame();
    currentFrame = (currentFrame + 1) % 2;
}

void VulkanRenderer::renderTriangle(const glm::vec3& position, const glm::vec3& color) {
    // 簡単な三角形の描画（現在はプレースホルダー）
    std::cout << "Rendering triangle at (" << position.x << ", " << position.y << ", " << position.z 
              << ") with color (" << color.x << ", " << color.y << ", " << color.z << ")" << std::endl;
}

void VulkanRenderer::renderCube(const glm::vec3& position, const glm::vec3& color) {
    // 簡単な立方体の描画（現在はプレースホルダー）
    std::cout << "Rendering cube at (" << position.x << ", " << position.y << ", " << position.z 
              << ") with color (" << color.x << ", " << color.y << ", " << color.z << ")" << std::endl;
}

void VulkanRenderer::renderText(const std::string& text, const glm::vec2& position, const glm::vec3& color) {
    // 簡単なテキストの描画（現在はプレースホルダー）
    std::cout << "Rendering text '" << text << "' at (" << position.x << ", " << position.y 
              << ") with color (" << color.x << ", " << color.y << ", " << color.z << ")" << std::endl;
}

void VulkanRenderer::recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
        std::cerr << "Failed to begin recording command buffer" << std::endl;
        return;
    }

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = renderPass;
    renderPassInfo.framebuffer = swapChainFramebuffers[imageIndex];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = swapChainExtent;

    VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdEndRenderPass(commandBuffer);

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        std::cerr << "Failed to record command buffer" << std::endl;
    }
}

void VulkanRenderer::drawFrame() {
    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(device->getDevice(), swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        return;
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        std::cerr << "Failed to acquire swap chain image" << std::endl;
        return;
    }

    recordCommandBuffer(commandBuffers[currentFrame], imageIndex);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffers[currentFrame];

    VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    if (vkQueueSubmit(device->getGraphicsQueue(), 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) {
        std::cerr << "Failed to submit draw command buffer" << std::endl;
        return;
    }

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = {swapChain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;

    result = vkQueuePresentKHR(device->getPresentQueue(), &presentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        return;
    } else if (result != VK_SUCCESS) {
        std::cerr << "Failed to present swap chain image" << std::endl;
        return;
    }
}

} // namespace gfx
