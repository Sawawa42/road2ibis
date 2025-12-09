#include <GLES3/gl32.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <cstdint>

#include "Shader.hpp"
#include "Mesh.hpp"
#include "FrameBuffer.hpp"

const char* vertexShaderSource = R"(#version 300 es
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTexCoord;
out vec2 TexCoord;
uniform vec2 uScale;
void main() {
    gl_Position = vec4(aPos.x * uScale.x, aPos.y * uScale.y, aPos.z, 1.0);
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

void glClearColorUint8(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    glClearColor(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);
}

int main() {
    if (!glfwInit()) {
        return -1;
    }
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);

    GLFWwindow* window = glfwCreateWindow(800, 600, "tinyPaint", nullptr, nullptr);
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
        glClearColorUint8(255, 0, 0, 255); // カラーバッファのクリア値を赤色に設定
        glClear(GL_COLOR_BUFFER_BIT);
        canvas.unbind();
    }

    while (!glfwWindowShouldClose(window)) {
        glClearColorUint8(255, 255, 255, 255); // カラーバッファのクリア値を白色に設定
        glClear(GL_COLOR_BUFFER_BIT);

        shader.use();

        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);

        float scaleX = 1.0f;
        float scaleY = 1.0f;

        if (width > height) {
            scaleX = (float)height / (float)width;
        } else {
            scaleY = (float)width / (float)height;
        }
        int scaleLoc = glGetUniformLocation(shader.ID, "uScale");
        glUniform2f(scaleLoc, scaleX, scaleY);

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
