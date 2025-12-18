#include "LayerTexture.hpp"

LayerTexture::LayerTexture(int width, int height)
    : width(width), height(height) {
    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    glBindTexture(GL_TEXTURE_2D, 0);
}

LayerTexture::~LayerTexture() {
    if (textureId != 0) {
        glDeleteTextures(1, &textureId);
    }
}

LayerTexture::LayerTexture(LayerTexture&& other) noexcept
    : textureId(other.textureId), width(other.width), height(other.height) {
    other.textureId = 0;
    other.width = 0;
    other.height = 0;
}

LayerTexture& LayerTexture::operator=(LayerTexture&& other) noexcept {
    if (this != &other) {
        if (textureId != 0) {
            glDeleteTextures(1, &textureId);
        }
        textureId = other.textureId;
        width = other.width;
        height = other.height;
        other.textureId = 0;
        other.width = 0;
        other.height = 0;
    }
    return *this;
}

void LayerTexture::updateTile(int x, int y, int tileWidth, int tileHeight, const uint8_t* data) {
    glBindTexture(GL_TEXTURE_2D, textureId);
    glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, tileWidth, tileHeight,
                    GL_RGBA, GL_UNSIGNED_BYTE, data);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void LayerTexture::clear(float r, float g, float b, float a) {
    // 一時的なFBOを使用してクリア
    GLuint tempFbo;
    glGenFramebuffers(1, &tempFbo);
    glBindFramebuffer(GL_FRAMEBUFFER, tempFbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureId, 0);

    glViewport(0, 0, width, height);
    glClearColor(r, g, b, a);
    glClear(GL_COLOR_BUFFER_BIT);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteFramebuffers(1, &tempFbo);
}

std::vector<uint8_t> LayerTexture::readAllPixels() const {
    std::vector<uint8_t> pixels(width * height * 4);

    // 一時的なFBOを使用して読み取り
    GLuint tempFbo;
    glGenFramebuffers(1, &tempFbo);
    glBindFramebuffer(GL_FRAMEBUFFER, tempFbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureId, 0);

    glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteFramebuffers(1, &tempFbo);

    return pixels;
}
