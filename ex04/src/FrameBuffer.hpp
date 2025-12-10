#pragma once
#include <GLES3/gl32.h>
#include <iostream>

class FrameBuffer {
public:
    unsigned int fboId;
    unsigned int textureId;
    int width, height;

    FrameBuffer(int w, int h);
    ~FrameBuffer();

    void bind();
    void unbind();

    unsigned int getTexture() const;
};
