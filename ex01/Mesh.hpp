#pragma once
#include <GLES3/gl32.h>
#include <vector>

class Mesh {
    public:
        Mesh(const std::vector<float>& vertices);

        void draw() const;

        ~Mesh();
    
    private:
        unsigned int VAO, VBO;
        int vertexCount;
};
