#pragma once
#include <GL/glew.h>
#include <string>
#include <iostream>

class Shader {
    public:
        unsigned int ID; // Shader program ID

        // コンストラクタでソースコードを受けとり、ビルドする
        Shader(const char* vertexSource, const char* fragmentSource);

        // シェーダー有効化
        void use() const;

        // デストラクタでシェーダーを削除
        ~Shader();
    
    private:
        void checkCompileErrors(unsigned int shader, std::string type);
};
