#pragma once
#include <GL/glew.h>
#include <string>
#include <iostream>
#include <fstream>
#include <vector>

class Shader {
    public:
        unsigned int ID; // Shader program ID

        // コンストラクタでソースコードを受けとり、ビルドする
        Shader(const char* vertexSource, const char* fragmentSource, const std::string& cacheFilePath);

        // シェーダー有効化
        void use() const;

        // デストラクタでシェーダーを削除
        ~Shader();
    
    private:
        bool loadFromBinary(const std::string& cacheFilePath);
        void compileFromSource(const char* vertexSource, const char* fragmentSource);
        void saveBinary(const std::string& cacheFilePath);
        void checkCompileErrors(unsigned int shader, std::string type);
};
