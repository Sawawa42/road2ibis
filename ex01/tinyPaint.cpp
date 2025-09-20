#include "tinyPaint.hpp"

TinyPaint::TinyPaint(int width, int height, const char *title)
    : _width(width), _height(height), _title(title) {}

TinyPaint::~TinyPaint() {
    cleanup();
}

const char* vertexShaderSource = R"(#version 320 es
layout (location = 0) in vec3 aPos;
void main() {
    gl_Position = vec4(aPos, 1.0);
})";

const char* fragmentShaderSource = R"(#version 320 es
precision mediump float;
uniform vec4 ourColor; 
out vec4 FragColor;
void main() {
    FragColor = ourColor;
})";

// シェーダーのコンパイルとチェックを行うヘルパー関数
GLuint compileShader(GLuint type, const char* source) {
    GLuint shader = gl.glCreateShader(type);
    gl.glShaderSource(shader, 1, &source, NULL);
    gl.glCompileShader(shader);

    // エラーチェック
    GLint success;
    gl.glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        throw std::runtime_error("Shader compilation failed");
    }
    return shader;
}

int TinyPaint::init() {
    if (!glfwInit()) {
        return -1;
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);

    this->_window = glfwCreateWindow(this->_width, this->_height, this->_title, NULL, NULL);
    if (!this->_window) {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(this->_window);
    gl.load();

    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

    this->_shaderProgram = gl.glCreateProgram();
    gl.glAttachShader(this->_shaderProgram, vertexShader);
    gl.glAttachShader(this->_shaderProgram, fragmentShader);
    gl.glLinkProgram(this->_shaderProgram);

    this->_ourColorLocation = gl.glGetUniformLocation(this->_shaderProgram, "ourColor");

    GLint success;
    gl.glGetProgramiv(this->_shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        // throw std::runtime_error("Program linking failed");
        return -1;
    }
    gl.glDeleteShader(vertexShader);
    gl.glDeleteShader(fragmentShader);

    gl.glGenVertexArrays(1, &this->_VAO);
    gl.glGenBuffers(1, &this->_VBO);

    gl.glBindVertexArray(this->_VAO);
    gl.glBindBuffer(GL_ARRAY_BUFFER, this->_VBO);
    gl.glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 9, nullptr, GL_DYNAMIC_DRAW);

    gl.glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    gl.glEnableVertexAttribArray(0);

    gl.glBindBuffer(GL_ARRAY_BUFFER, 0);
    gl.glBindVertexArray(0);

    return 0;
}

void TinyPaint::run() {
    while (!glfwWindowShouldClose(this->_window)) {
        int display_w, display_h;
        glfwGetFramebufferSize(this->_window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);

        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // 描画コードをここに追加
        float vertices[] = {
            -0.5f, -0.5f, 0.0f,
             0.5f, -0.5f, 0.0f,
             0.0f,  0.5f, 0.0f
        };

        gl.glBindBuffer(GL_ARRAY_BUFFER, this->_VBO);
        gl.glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        gl.glBindBuffer(GL_ARRAY_BUFFER, 0);

        gl.glUseProgram(this->_shaderProgram);
        gl.glUniform4f(this->_ourColorLocation, 1.0f, 0.0f, 0.0f, 1.0f); // 赤色
        gl.glBindVertexArray(this->_VAO);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        glfwSwapBuffers(this->_window);
        glfwPollEvents();
    }
}

void TinyPaint::cleanup() {
    gl.glDeleteVertexArrays(1, &this->_VAO);
    gl.glDeleteBuffers(1, &this->_VBO);
    gl.glDeleteProgram(this->_shaderProgram);
    if (_window) {
        glfwDestroyWindow(_window);
    }
    glfwTerminate();
}
