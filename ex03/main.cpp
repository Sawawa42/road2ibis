#include <GLES3/gl32.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <cstdint>

#include "Shader.hpp"
#include "Mesh.hpp"
#include "FrameBuffer.hpp"
#include "Brush.hpp"

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
    // GLFW初期化
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

    // 透過処理を有効化
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    Shader shader(vertexShaderSource, fragmentShaderSource);

    std::vector<float> vertices = {
        -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, // 左上
        -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, // 左下
        1.0f, 1.0f, 0.0f, 1.0f, 1.0f,  // 右上
        1.0f, -1.0f, 0.0f, 1.0f, 0.0f, // 右下
    };
    Mesh mesh(vertices, MeshFormat::XYZ_UV);

    float fboSize = 4096.0f;
    FrameBuffer canvas((int)fboSize, (int)fboSize);

    // ブラシの作成
    Brush brush;
    brush.setColor(0, 0, 0); // 黒色
    brush.setSize(30.0f);

    {
        canvas.bind();
        glViewport(0, 0, (int)fboSize, (int)fboSize);
        glClearColorUint8(255, 0, 0, 128); // カラーバッファのクリア値を半透明な赤色に設定
        glClear(GL_COLOR_BUFFER_BIT);
        canvas.unbind();
    }

    bool isDrawing = false;
    float lastX = 0.0f, lastY = 0.0f;

    while (!glfwWindowShouldClose(window)) {
        // 画面サイズ取得、スケール計算
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        float scaleX = 1.0f;
        float scaleY = 1.0f;

        if (width > height) {
            scaleX = (float)height / (float)width;
        } else {
            scaleY = (float)width / (float)height;
        }

        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
            double mouseX, mouseY;
            glfwGetCursorPos(window, &mouseX, &mouseY);

            // 1. ウィンドウ座標 -> NDC (-1 ~ 1)
            // Y軸は反転させる
            float ndcX = ((float)mouseX / width) * 2.0f - 1.0f;
            float ndcY = 1.0f - ((float)mouseY / height) * 2.0f;

            // 2. スケール逆補正 -> キャンバス上のNDC
            float canvasX = ndcX / scaleX;
            float canvasY = ndcY / scaleY;

            // 3. キャンバス内か判定
            bool inside = (canvasX >= -1.0f && canvasX <= 1.0f && canvasY >= -1.0f && canvasY <= 1.0f);
            if (inside) {
                canvas.bind();
                glViewport(0, 0, (int)fboSize, (int)fboSize);
                brush.begin();
                if (isDrawing) {
                    brush.drawLine(lastX, lastY, canvasX, canvasY, fboSize);
                } else {
                    brush.drawLine(canvasX, canvasY, canvasX, canvasY, fboSize);
                    isDrawing = true;
                }
                canvas.unbind();
            }
            lastX = canvasX;
            lastY = canvasY;
            isDrawing = true;
        } else {
            isDrawing = false;
        }

        // --- 表示処理 ---
        glViewport(0, 0, width, height);
        glClearColorUint8(255, 255, 255, 255); // カラーバッファのクリア値を白色に設定
        glClear(GL_COLOR_BUFFER_BIT);
        shader.use();

        glUniform2f(glGetUniformLocation(shader.ID, "uScale"), scaleX, scaleY);

        // FBOのテクスチャをバインド
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, canvas.getTexture());
        glUniform1i(glGetUniformLocation(shader.ID, "uTexture"), 0);

        mesh.draw(GL_TRIANGLE_STRIP);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
