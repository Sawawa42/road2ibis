#include "Renderer.hpp"
#include <vector>

const char* vertexShaderSource = R"(#version 410 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTexCoord;
out vec2 TexCoord;
uniform vec2 uScale;
void main() {
    gl_Position = vec4(aPos.x * uScale.x, aPos.y * uScale.y, aPos.z, 1.0);
    TexCoord = aTexCoord;
})";

const char* fragmentShaderSource = R"(#version 410 core
in vec2 TexCoord;
uniform sampler2D uTexture;
out vec4 FragColor;
void main() {
    vec4 texColor = texture(uTexture, TexCoord);

    vec3 bgColor = vec3(1.0, 1.0, 1.0); // 白背景
    vec3 blendedColor = mix(bgColor, texColor.rgb, texColor.a);

    FragColor = vec4(blendedColor, 1.0);
})";

Renderer::Renderer() {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    shader = std::make_unique<Shader>(vertexShaderSource, fragmentShaderSource, "rendererShader.bin");

    std::vector<float> vertices = {
        -1.0f, 1.0f, 0.0f, 0.0f, 1.0f,  // 左上
        -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, // 左下
        1.0f, 1.0f, 0.0f, 1.0f, 1.0f,   // 右上
        1.0f, -1.0f, 0.0f, 1.0f, 0.0f,  // 右下
    };
    mesh = std::make_unique<Mesh>(vertices, MeshFormat::XYZ_UV);
}

void Renderer::clear(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    glClearColor(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}

void Renderer::setViewport(int width, int height) {
    glViewport(0, 0, width, height);
}

void Renderer::renderCanvas(GLuint texture, float scaleX, float scaleY) {
    shader->use();
    glUniform2f(glGetUniformLocation(shader->ID, "uScale"), scaleX, scaleY);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glUniform1i(glGetUniformLocation(shader->ID, "uTexture"), 0);

    mesh->draw(GL_TRIANGLE_STRIP);
}
