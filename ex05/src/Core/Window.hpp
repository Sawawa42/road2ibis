#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>

class Window {
public:
    Window(int width, int height, const char* title);
    ~Window();

    bool shouldClose() const;
    void swapBuffers();
    void pollEvents();

    void getFramebufferSize(int& width, int& height) const;
    GLFWwindow* getHandle() const { return window; }

private:
    GLFWwindow* window;
};
