#pragma once
#include <GL/glew.h>
#include <vector>
#include <cmath>
#include "Shader.hpp"
#include "Mesh.hpp"

class Brush {
    public:
        Brush();
        ~Brush();

        void setColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a);

        void setSize(float px);

        void begin();

        void drawLine(float startX, float startY, float endX, float endY, float fboWidth);
    
    private:
        Shader* shader;
        Mesh* mesh;
        float color[4];
        float size;

        void drawPoint(float x, float y, float sizeNDC);
};
