#pragma once
#include <GLES3/gl32.h>
#include <GLFW/glfw3.h>
#include <memory> // unique_ptr

#include "Shader.hpp"
#include "Mesh.hpp"
#include "FrameBuffer.hpp"
#include "Brush.hpp"

class App {
    public:
        App(int width, int height, const char* title, float size);
        ~App();

        void run();

        void saveImage(const char* filename);

    private:
        GLFWwindow* window;

        std::unique_ptr<Shader> shader;
        std::unique_ptr<Mesh> mesh;
        std::unique_ptr<FrameBuffer> canvas;
        std::unique_ptr<Brush> brush;

        float fboSize;
        bool isDrawing;
        float lastX, lastY;

        void initOpenGL();
        void processInput(int width, int height, float scaleX, float scaleY);
        void render(int width, int height, float scaleX, float scaleY);

        void clearColorUint8(uint8_t r, uint8_t g, uint8_t b, uint8_t a);

        // ダブルバッファリング用に2つのPBOを用意
        GLuint pboIds[2];

        // 今読み出し命令を出しているPBOのインデックス
        int pboIndex = 0;

        // 次に読み出し命令を出すPBOのインデックス
        int nextPboIndex = 1;

        const int tileSize = 128; // px
        const int channels = 4; // RGBA

        void initPBOs();

        void processPBO(int x, int y);
};
