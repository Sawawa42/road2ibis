#pragma once
#include <GLES3/gl32.h>
#include <memory>
#include "Graphics/Shader.hpp"
#include "Graphics/Mesh.hpp"

class Renderer {
public:
    Renderer();
    ~Renderer() = default;

    void clear(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
    void renderCanvas(GLuint texture, float scaleX, float scaleY);

    void setViewport(int width, int height);

private:
    std::unique_ptr<Shader> shader;
    std::unique_ptr<Mesh> mesh;
};
