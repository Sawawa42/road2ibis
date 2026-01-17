#pragma once
#include <GL/glew.h>
#include <iostream>

class FrameBuffer {
public:
    unsigned int fboId;
    unsigned int textureId;
    int width, height;
    bool ownsTexture;  // テクスチャを所有するか

    // 内部でテクスチャを作成
    FrameBuffer(int w, int h);

    // 外部テクスチャをアタッチ（テクスチャは所有しない）
    FrameBuffer(GLuint externalTexture);

    ~FrameBuffer();

    void bind();
    void unbind();

    unsigned int getTexture() const;
};
