#pragma once
#include <GLES3/gl32.h>
#include <vector>

enum class MeshFormat {
    XYZ_UV,
    XY
};

class Mesh {
    public:
        Mesh(const std::vector<float>& vertices, MeshFormat format);

        void draw(GLenum mode = GL_TRIANGLES) const;

        ~Mesh();
    
    private:
        unsigned int VAO, VBO;
        int vertexCount;
};
