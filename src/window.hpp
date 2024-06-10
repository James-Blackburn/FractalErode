#ifndef WINDOW_HPP_INCLUDED
#define WINDOW_HPP_INCLUDED

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <unordered_map>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

class Window{
private:
    GLFWwindow* glfwWindow = nullptr;
    bool closing = false;
public:
    const char* title = "window";
    unsigned int width = 1280; 
    unsigned int height = 720;
    float mx = 640.0f;
    float my = 360.0f;
    float scroll = 0.0f;
    bool fullscreen = false;
    
    std::unordered_map<unsigned int, bool> keys;
    std::unordered_map<unsigned int, bool> mouseButtons;

    Window() {};
    Window(const Window&) = delete;
    int init(int, int, const char*, bool);
    void clean();

    inline void setWindowShouldClose(bool value);
    inline bool windowShouldClose();
    inline static Window* getInstance();
    inline void update();
    inline void showCursor();
    inline void hideCursor();
};

void Window::setWindowShouldClose(bool value) {
    closing = value;
    glfwSetWindowShouldClose(glfwWindow, value);
}

bool Window::windowShouldClose() {
    return closing;
}

void Window::update() {
    glfwSwapBuffers(glfwWindow);
}

Window* Window::getInstance() {
    static Window window;
    return &window;
}

void Window::showCursor() {
    glfwSetInputMode(glfwWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}

void Window::hideCursor() {
    glfwSetInputMode(glfwWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

#endif