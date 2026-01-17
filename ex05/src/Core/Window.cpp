#include "Window.hpp"
#include <cstdlib>

Window::Window(int width, int height, const char* title) {
    if (!glfwInit()) {
        exit(-1);
    }
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);

    window = glfwCreateWindow(width, height, title, nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        exit(-1);
    }
    glfwMakeContextCurrent(window);

    glewInit();
}

Window::~Window() {
    glfwDestroyWindow(window);
    glfwTerminate();
}

bool Window::shouldClose() const {
    return glfwWindowShouldClose(window);
}

void Window::swapBuffers() {
    glfwSwapBuffers(window);
}

void Window::pollEvents() {
    glfwPollEvents();
}

void Window::getFramebufferSize(int& width, int& height) const {
    glfwGetFramebufferSize(window, &width, &height);
}
