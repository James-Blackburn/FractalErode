#include "window.hpp"
#include <iostream>

void framebufferSizeCallback(GLFWwindow*, int, int);
void keyCallback(GLFWwindow*, int, int, int, int);
void cursorPositionCallback(GLFWwindow*, double, double);
void mouseButtonCallback(GLFWwindow*, int, int, int);
void scollCallback(GLFWwindow*, double, double);

int Window::init(int aWidth, int aHeight, const char* aTitle, bool isFullscreen){
    width = aWidth;
    height = aHeight;
    mx = (float)width / 2.0f;
    my = (float)height / 2.0f;
    title = aTitle;
    fullscreen = isFullscreen;

    // initialize and cofigure glfw
    if (!glfwInit()) {
        std::cout << "GLFW initialization failed" << std::endl;
        return -1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);

    // create window with glfw
    if (fullscreen){
        GLFWmonitor* monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);
        glfwWindow = glfwCreateWindow(mode->width, mode->height, title, monitor, NULL);
    } else{
        glfwWindow = glfwCreateWindow(width, height, title, NULL, NULL);
    }
    if (glfwWindow == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    // make the OpenGL context current on this thread
    glfwMakeContextCurrent(glfwWindow);
    // set the glfw user pointer to this instance of the window struct
    glfwSetWindowUserPointer(glfwWindow, this);

    // set glfw callbacks
    glfwSetFramebufferSizeCallback(glfwWindow, framebufferSizeCallback);
    glfwSetKeyCallback(glfwWindow, keyCallback);
    glfwSetCursorPosCallback(glfwWindow, cursorPositionCallback);
    glfwSetMouseButtonCallback(glfwWindow, mouseButtonCallback);
    glfwSetScrollCallback(glfwWindow, scollCallback);

    // disable window resizing as it messes up UI
    glfwSetWindowAttrib(glfwWindow, GLFW_RESIZABLE, GLFW_FALSE);

    // load OpenGL function pointers with glad
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // enable certain OpenGL features
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    // enable vsync
    glfwSwapInterval(1);

    // Setup ImGui context and platform backends
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(glfwWindow, true);
    ImGui_ImplOpenGL3_Init();

    return 0;
}

void Window::clean() {
    // Clean resources
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(glfwWindow);
    glfwTerminate();
}

// GLFW event callbacks
void framebufferSizeCallback(GLFWwindow* glfwwindow, int aWidth, int aHeight){
    Window* window = (Window*)glfwGetWindowUserPointer(glfwwindow);
    window->width = aWidth;
    window->height = aHeight;
    glViewport(0, 0, window->width, window->height);
}

void keyCallback(GLFWwindow* glfwwindow, int key, int scancode, int action, int mods){
    Window* window = (Window*)glfwGetWindowUserPointer(glfwwindow);
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS){
        window->setWindowShouldClose(true);
    }
    if (action == GLFW_PRESS){
        window->keys[key] = true;
    } else if (action == GLFW_RELEASE){
        window->keys[key] = false;
    }
}

void cursorPositionCallback(GLFWwindow* glfwwindow, double x, double y){
    Window* window = (Window*)glfwGetWindowUserPointer(glfwwindow);
    window->mx = (float)x;
    window->my = (float)y;
}

void mouseButtonCallback(GLFWwindow* glfwwindow, int button, int action, int mods) {
    ((Window*)glfwGetWindowUserPointer(glfwwindow))->mouseButtons[button] = action;
}

void scollCallback(GLFWwindow* glfwwindow, double xoffset, double yoffset)
{
    ((Window*)glfwGetWindowUserPointer(glfwwindow))->scroll = yoffset;
}