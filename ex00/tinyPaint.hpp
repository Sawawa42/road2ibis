#pragma once
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>

class TinyPaint {
    public:
        TinyPaint(int width, int height, const char *title);
        ~TinyPaint();

        int init();
        void run();
        void cleanup();
    private:
        GLFWwindow* _window;
        GLuint _shaderProgram;
        GLuint _VAO;
        GLuint _VBO;
        int    _ourColorLocation;

        int _width;
        int _height;
        const char* _title;
};

// 42Tokyo校舎環境にGLEWが入らなかったため、手動読み込みを行う
struct OpenGLFunctions {
    PFNGLCREATESHADERPROC glCreateShader;
    PFNGLSHADERSOURCEPROC glShaderSource;
    PFNGLCOMPILESHADERPROC glCompileShader;
    PFNGLGETSHADERIVPROC glGetShaderiv;
    PFNGLCREATEPROGRAMPROC glCreateProgram;
    PFNGLATTACHSHADERPROC glAttachShader;
    PFNGLLINKPROGRAMPROC glLinkProgram;
    PFNGLGETPROGRAMIVPROC glGetProgramiv;
    PFNGLDELETESHADERPROC glDeleteShader;
    PFNGLUSEPROGRAMPROC glUseProgram;
    PFNGLGENVERTEXARRAYSPROC glGenVertexArrays;
    PFNGLBINDVERTEXARRAYPROC glBindVertexArray;
    PFNGLGENBUFFERSPROC glGenBuffers;
    PFNGLBINDBUFFERPROC glBindBuffer;
    PFNGLBUFFERDATAPROC glBufferData;
    PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer;
    PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray;
    PFNGLDELETEPROGRAMPROC glDeleteProgram;
    PFNGLDELETEVERTEXARRAYSPROC glDeleteVertexArrays;
    PFNGLDELETEBUFFERSPROC glDeleteBuffers;
    PFNGLBUFFERSUBDATAPROC glBufferSubData;
    PFNGLUNIFORM4FPROC glUniform4f;
    PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation;

    void load() {
        glCreateShader = (PFNGLCREATESHADERPROC)glfwGetProcAddress("glCreateShader");
        glShaderSource = (PFNGLSHADERSOURCEPROC)glfwGetProcAddress("glShaderSource");
        glCompileShader = (PFNGLCOMPILESHADERPROC)glfwGetProcAddress("glCompileShader");
        glGetShaderiv = (PFNGLGETSHADERIVPROC)glfwGetProcAddress("glGetShaderiv");
        glCreateProgram = (PFNGLCREATEPROGRAMPROC)glfwGetProcAddress("glCreateProgram");
        glAttachShader = (PFNGLATTACHSHADERPROC)glfwGetProcAddress("glAttachShader");
        glLinkProgram = (PFNGLLINKPROGRAMPROC)glfwGetProcAddress("glLinkProgram");
        glGetProgramiv = (PFNGLGETPROGRAMIVPROC)glfwGetProcAddress("glGetProgramiv");
        glDeleteShader = (PFNGLDELETESHADERPROC)glfwGetProcAddress("glDeleteShader");
        glUseProgram = (PFNGLUSEPROGRAMPROC)glfwGetProcAddress("glUseProgram");
        glGenVertexArrays = (PFNGLGENVERTEXARRAYSPROC)glfwGetProcAddress("glGenVertexArrays");
        glBindVertexArray = (PFNGLBINDVERTEXARRAYPROC)glfwGetProcAddress("glBindVertexArray");
        glGenBuffers = (PFNGLGENBUFFERSPROC)glfwGetProcAddress("glGenBuffers");
        glBindBuffer = (PFNGLBINDBUFFERPROC)glfwGetProcAddress("glBindBuffer");
        glBufferData = (PFNGLBUFFERDATAPROC)glfwGetProcAddress("glBufferData");
        glVertexAttribPointer = (PFNGLVERTEXATTRIBPOINTERPROC)glfwGetProcAddress("glVertexAttribPointer");
        glEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC)glfwGetProcAddress("glEnableVertexAttribArray");
        glDeleteProgram = (PFNGLDELETEPROGRAMPROC)glfwGetProcAddress("glDeleteProgram");
        glDeleteVertexArrays = (PFNGLDELETEVERTEXARRAYSPROC)glfwGetProcAddress("glDeleteVertexArrays");
        glDeleteBuffers = (PFNGLDELETEBUFFERSPROC)glfwGetProcAddress("glDeleteBuffers");
        glBufferSubData = (PFNGLBUFFERSUBDATAPROC)glfwGetProcAddress("glBufferSubData");
        glUniform4f = (PFNGLUNIFORM4FPROC)glfwGetProcAddress("glUniform4f");
        glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC)glfwGetProcAddress("glGetUniformLocation");
    }
};

extern OpenGLFunctions gl;
