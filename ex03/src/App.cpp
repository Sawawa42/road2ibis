#include "App.hpp"
#include <iostream>
#include <vector>

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

App::App(int width, int height, const char* title, float size)
    : isDrawing(false), lastX(0.0f), lastY(0.0f), fboSize(size) {
    // GLFW初期化
    if (!glfwInit()) {
        exit(-1);
    }
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);

    window = glfwCreateWindow(width, height, title, nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        exit(-1);
    }
    glfwMakeContextCurrent(window);

    initOpenGL();
}

App::~App() {
    glfwDestroyWindow(window);
    glfwTerminate();
}

void App::initOpenGL() {
    // 透過処理を有効化
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    shader = std::make_unique<Shader>(vertexShaderSource, fragmentShaderSource);

    std::vector<float> vertices = {
        -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, // 左上
        -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, // 左下
        1.0f, 1.0f, 0.0f, 1.0f, 1.0f,  // 右上
        1.0f, -1.0f, 0.0f, 1.0f, 0.0f, // 右下
    };
    mesh = std::make_unique<Mesh>(vertices, MeshFormat::XYZ_UV);

    canvas = std::make_unique<FrameBuffer>((int)fboSize, (int)fboSize);
    brush = std::make_unique<Brush>();

    canvas->bind();
    glViewport(0, 0, (int)fboSize, (int)fboSize);
    clearColorUint8(255, 0, 0, 128); // 本当は透明とかが良いが、確認しやすいように着色
    glClear(GL_COLOR_BUFFER_BIT);
    canvas->unbind();
}

void App::run() {
    while (!glfwWindowShouldClose(window)) {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        float scaleX = 1.0f;
        float scaleY = 1.0f;

        if (width > height) {
            scaleX = (float)height / (float)width;
        } else {
            scaleY = (float)width / (float)height;
        }

        processInput(width, height, scaleX, scaleY);
        render(width, height, scaleX, scaleY);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}

void App::processInput(int width, int height, float scaleX, float scaleY) {
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
            canvas->bind();
            glViewport(0, 0, (int)fboSize, (int)fboSize);
            brush->begin();
            if (isDrawing) {
                brush->drawLine(lastX, lastY, canvasX, canvasY, fboSize);
            } else {
                brush->drawLine(canvasX, canvasY, canvasX, canvasY, fboSize);
                isDrawing = true;
            }
            canvas->unbind();
        }
        lastX = canvasX;
        lastY = canvasY;
        isDrawing = true;
    } else {
        isDrawing = false;
    }
}

void App::render(int width, int height, float scaleX, float scaleY) {
    glViewport(0, 0, width, height);
    clearColorUint8(255, 255, 255, 255);
    glClear(GL_COLOR_BUFFER_BIT);

    shader->use();
    glUniform2f(glGetUniformLocation(shader->ID, "uScale"), scaleX, scaleY);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, canvas->getTexture());
    glUniform1i(glGetUniformLocation(shader->ID, "uTexture"), 0);

    mesh->draw(GL_TRIANGLE_STRIP);
}

void App::clearColorUint8(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    glClearColor(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);
}
