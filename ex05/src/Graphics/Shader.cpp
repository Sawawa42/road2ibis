#include "Shader.hpp"

Shader::Shader(const char* vertexSource, const char* fragmentSource, const std::string& cacheFilePath) {
    bool loadSuccess = loadFromBinary(cacheFilePath);

    if (!loadSuccess) {
        compileFromSource(vertexSource, fragmentSource);
        saveBinary(cacheFilePath);
    }
}

bool Shader::loadFromBinary(const std::string& cacheFilePath) {
    std::ifstream ifs(cacheFilePath, std::ios::binary);

    ID = glCreateProgram();
    
    if (ifs.is_open()) {
        GLenum format;
        GLsizei length;

        ifs.read(reinterpret_cast<char *>(&format), sizeof(format));
        ifs.read(reinterpret_cast<char *>(&length), sizeof(length));

        if (length > 0) {
            std::vector<char> buffer(length);
            ifs.read(buffer.data(), length);

            glProgramBinary(ID, format, buffer.data(), length);

            GLint ok;
            glGetProgramiv(ID, GL_LINK_STATUS, &ok);

            if (ok) {
                std::cout << "Successfully loaded program from binary cache: " << cacheFilePath << std::endl;
                ifs.close();
                return true;
            }
        }
        ifs.close();
    }
    std::cout << "Cache file not found or unreadable. Compiling from source..." << std::endl;
    return false;
}

void Shader::compileFromSource(const char* vertexSource, const char* fragmentSource) {
    unsigned int vertex, fragment;

    // Vertex Shader
    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vertexSource, NULL);
    glCompileShader(vertex);
    checkCompileErrors(vertex, "VERTEX");

    // Fragment Shader
    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fragmentSource, NULL);
    glCompileShader(fragment);
    checkCompileErrors(fragment, "FRAGMENT");

    // Shader Program
    glAttachShader(ID, vertex);
    glAttachShader(ID, fragment);
    glLinkProgram(ID);
    checkCompileErrors(ID, "PROGRAM");

    // リンク後に個別のシェーダーオブジェクトは不要になるので削除
    glDeleteShader(vertex);
    glDeleteShader(fragment);
    std::cout << "Successfully compiled and linked program from source." << std::endl;
}

void Shader::saveBinary(const std::string& cacheFilePath) {
    GLint formats = 0;
    glGetIntegerv(GL_NUM_PROGRAM_BINARY_FORMATS, &formats);

    if (formats > 0) {
        GLint length = 0;
        glGetProgramiv(ID, GL_PROGRAM_BINARY_LENGTH, &length);

        if (length > 0) {
            std::vector<char> buffer(length);
            GLenum format = 0;

            glGetProgramBinary(ID, length, NULL, &format, buffer.data());

            std::ofstream ofs(cacheFilePath, std::ios::binary);
            if (ofs.is_open()) {
                ofs.write(reinterpret_cast<char *>(&format), sizeof(format));
                ofs.write(reinterpret_cast<char *>(&length), sizeof(length));
                ofs.write(buffer.data(), length);
                std::cout << "Shader binary successfully saved to: " << cacheFilePath << std::endl;
                ofs.close();
                return;
            }
        }
    }
    std::cout << "Binary saving skipped." << std::endl;
}

void Shader::use() const {
    glUseProgram(ID);
}

Shader::~Shader() {
    glDeleteProgram(ID);
}

void Shader::checkCompileErrors(unsigned int shader, std::string type) {
    int success;
    char infoLog[1024];

    if (type != "PROGRAM") {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(shader, 1024, NULL, infoLog);
            throw std::runtime_error("Shader compilation error: " + std::string(infoLog));
        }
    } else {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(shader, 1024, NULL, infoLog);
            throw std::runtime_error("Program linking error: " + std::string(infoLog));
        }
    }
}