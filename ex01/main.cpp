#include <GLES3/gl32.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>

#include "Shader.hpp"
#include "Mesh.hpp"
#include "FrameBuffer.hpp"

const char* vertexShaderSource = R"(#version 300 es
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTexCoord;
out vec2 TexCoord;
void main() {
    gl_Position = vec4(aPos, 1.0);
    TexCoord = aTexCoord;
})";

const char* fragmentShaderSource = R"(#version 300 es
precision mediump float;
in vec2 TexCoord;
uniform sampler2D uTexture;
out vec4 FragColor;
void main() {
    FragColor = texture(uTexture, TexCoord);
})";

int main() {
    if (!glfwInit()) {
        return -1;
    }
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);

    GLFWwindow* window = glfwCreateWindow(8000, 6000, "tinyPaint", nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    Shader shader(vertexShaderSource, fragmentShaderSource);

    std::vector<float> vertices = {
        -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, // 左上の頂点座標xyzとテクスチャ座標uv
        -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, // 左下の頂点座標xyzとテクスチャ座標uv
        1.0f, -1.0f, 0.0f, 1.0f, 0.0f, // 右下の頂点座標xyzとテクスチャ座標uv
        
        -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, // 左上の頂点座標xyzとテクスチャ座標uv
        1.0f, -1.0f, 0.0f, 1.0f, 0.0f, // 右下の頂点座標xyzとテクスチャ座標uv
        1.0f, 1.0f, 0.0f, 1.0f, 1.0f  // 右上の頂点座標xyzとテクスチャ座標uv
    };

    Mesh mesh(vertices);

    FrameBuffer canvas(4096, 4096);

    {
        canvas.bind();
        glClearColor(1.0f, 0.0f, 0.0f, 1.0f); // 赤色
        glClear(GL_COLOR_BUFFER_BIT);
        canvas.unbind();
    }

    while (!glfwWindowShouldClose(window)) {
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f); // 白色を背景色に設定
        glClear(GL_COLOR_BUFFER_BIT); // 画面をクリア

        shader.use();

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, canvas.getTexture());
        glUniform1i(glGetUniformLocation(shader.ID, "uTexture"), 0);

        mesh.draw();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
