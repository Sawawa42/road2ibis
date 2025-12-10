#include "Brush.hpp"

const char* brushVertexShaderSource = R"(#version 300 es
layout(location = 0) in vec2 aPos;
uniform vec2 uPos;
uniform vec2 uSize;

void main() {
    vec2 pos = uPos + (aPos * uSize);
    gl_Position = vec4(pos, 0.0, 1.0);
})";

const char* brushFragmentShaderSource = R"(#version 300 es
precision mediump float;
uniform vec4 uColor;
out vec4 FragColor;
void main() {
    FragColor = uColor;
})";

Brush::Brush() {
    // 1. シェーダー作成
    shader = new Shader(brushVertexShaderSource, brushFragmentShaderSource);
    
    // 2. 円形メッシュ作成 (XY形式)
    std::vector<float> vertices;
    vertices.push_back(0.0f); vertices.push_back(0.0f); // 中心
    
    int segments = 32;
    float radius = 0.5f;
    for (int i = 0; i <= segments; ++i) {
        float theta = 2.0f * 3.1415926f * float(i) / float(segments);
        vertices.push_back(radius * cos(theta));
        vertices.push_back(radius * sin(theta));
    }
    
    // フォーマットXYを指定してMesh作成
    mesh = new Mesh(vertices, MeshFormat::XY);

    // デフォルト設定
    color[0] = 0.0f; color[1] = 0.0f; color[2] = 0.0f; color[3] = 1.0f; // 黒
    size = 30.0f;
}

Brush::~Brush() {
    delete shader;
    delete mesh;
}

void Brush::setColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    color[0] = r / 255.0f;
    color[1] = g / 255.0f;
    color[2] = b / 255.0f;
    color[3] = a / 255.0f;
}

void Brush::setSize(float px) {
    size = px;
}

void Brush::begin() {
    shader->use();
    glUniform4f(glGetUniformLocation(shader->ID, "uColor"), color[0], color[1], color[2], color[3]);
}

float lerp(float a, float b, float f) {
    return a + f * (b - a);
}

void Brush::drawLine(float x1, float y1, float x2, float y2, float fboWidth) {
    // ブラシサイズをNDCに変換
    float sizeNDC = (size / fboWidth) * 2.0f;
    glUniform2f(glGetUniformLocation(shader->ID, "uSize"), sizeNDC, sizeNDC);

    // 距離計算
    float dist = sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2));
    
    // 補間ステップ数の計算
    int steps = (int)(dist / (sizeNDC * 0.1f)); 
    if (steps < 1) steps = 1;

    for (int i = 0; i <= steps; i++) {
        float t = (float)i / steps;
        float drawX = lerp(x1, x2, t);
        float drawY = lerp(y1, y2, t);
        
        drawPoint(drawX, drawY, sizeNDC);
    }
}

void Brush::drawPoint(float x, float y, float sizeNDC) {
    glUniform2f(glGetUniformLocation(shader->ID, "uPos"), x, y);
    // Meshクラスのdrawを呼ぶ (FANモード)
    mesh->draw(GL_TRIANGLE_FAN);
}