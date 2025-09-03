#pragma once

#include <iostream>
#include <string>

class ErrorHandler {
public:
    static bool handleGLFWError(const std::string& operation) {
        std::cerr << "GLFW Error during " << operation << std::endl;
        return false;
    }
    
    static bool handleRendererError(const std::string& operation) {
        std::cerr << "Renderer Error during " << operation << std::endl;
        return false;
    }
    
    static void logError(const std::string& message) {
        std::cerr << "ERROR: " << message << std::endl;
    }
    
    static void logInfo(const std::string& message) {
        std::cout << "INFO: " << message << std::endl;
    }
    
    static void logDebug(const std::string& message) {
        std::cout << "DEBUG: " << message << std::endl;
    }
};
