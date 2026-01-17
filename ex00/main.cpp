#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>

#include "Shader.hpp"
#include "Mesh.hpp"

const char* vertexShaderSource = R"(#version 460 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aColor;
out vec3 ourColor;
void main() {
    gl_Position = vec4(aPos, 1.0);
    ourColor = aColor;
})";

const char* fragmentShaderSource = R"(#version 460 core
in vec3 ourColor;
out vec4 FragColor;
void main() {
    FragColor = vec4(ourColor, 1.0);
})";

int main() {
    if (!glfwInit()) {
        return -1;
    }
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);

    GLFWwindow* window = glfwCreateWindow(800, 600, "tinyPaint", nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    glewInit();

    Shader shader(vertexShaderSource, fragmentShaderSource);

    std::vector<float> vertices = {
         0.0f,  0.5f, 0.0f,  1.0f, 0.0f, 0.0f, // 頂点1の座標xyzと色RGB
         0.5f, -0.5f, 0.0f,  1.0f, 0.0f, 0.0f, // 頂点2の座標xyzと色RGB
        -0.5f, -0.5f, 0.0f,  1.0f, 0.0f, 0.0f // 頂点3の座標xyzと色RGB
    };

    Mesh mesh(vertices);

    while (!glfwWindowShouldClose(window)) {
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f); // 白色を背景色に設定
        glClear(GL_COLOR_BUFFER_BIT); // 画面をクリア

        shader.use();
        mesh.draw();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
