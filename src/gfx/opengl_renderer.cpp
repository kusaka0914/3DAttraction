#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "opengl_renderer.h"
#include "../core/constants/game_constants.h"
#include <iostream>
#include <glm/gtc/type_ptr.hpp>

namespace gfx {

OpenGLRenderer::OpenGLRenderer() : window(nullptr) {
}

OpenGLRenderer::~OpenGLRenderer() {
    cleanup();
}

bool OpenGLRenderer::initialize(GLFWwindow* window) {
    this->window = window;
    
    glfwMakeContextCurrent(window);
    
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);
    
    glEnable(GL_DEPTH_TEST);
    
    glEnable(GL_TEXTURE_2D);
    
    font.initialize();
    
    return true;
}

void OpenGLRenderer::cleanup() {
    TextureManager::cleanup();
}

void OpenGLRenderer::beginFrame() {
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);
    
    glClearColor(GameConstants::RenderConstants::DEFAULT_BACKGROUND_COLOR.r, 
                 GameConstants::RenderConstants::DEFAULT_BACKGROUND_COLOR.g, 
                 GameConstants::RenderConstants::DEFAULT_BACKGROUND_COLOR.b, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(glm::value_ptr(projectionMatrix));
    
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(glm::value_ptr(viewMatrix));
}

void OpenGLRenderer::beginFrameWithBackground(int stageNumber) {
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    if (stageNumber == 6) {
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        backgroundRenderer.renderStageBackground(stageNumber);
    } else {
        backgroundRenderer.renderStageBackground(stageNumber);
    }
    
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(glm::value_ptr(projectionMatrix));
    
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(glm::value_ptr(viewMatrix));
}

void OpenGLRenderer::endFrame() {
    glfwSwapBuffers(window);
}

void OpenGLRenderer::setCamera(const glm::vec3& position, const glm::vec3& target, const glm::vec3& up) {
    viewMatrix = glm::lookAt(position, target, up);
}

void OpenGLRenderer::setProjection(float fov, float aspect, float near, float far) {
    projectionMatrix = glm::perspective(glm::radians(fov), aspect, near, far);
}
} // namespace gfx
