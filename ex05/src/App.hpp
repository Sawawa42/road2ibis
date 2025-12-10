#pragma once
#include <GLES3/gl32.h>
#include <GLFW/glfw3.h>
#include <memory> // unique_ptr
#include <set>
#include <algorithm>

#include "Shader.hpp"
#include "Mesh.hpp"
#include "FrameBuffer.hpp"
#include "Brush.hpp"
#include "UndoSystem.hpp"

struct TileCoord {
    int x, y;

    bool operator<(const TileCoord& other) const {
        if (x != other.x) {
            return x < other.x;
        }
        return y < other.y;
    }
};

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

        GLuint pboIds[16];

        int pboHead = 0;
        int pboTail = 0;
        int pendingPBOs = 0;

        const int tileSize = 128; // px
        const int channels = 4; // RGBA

        void initPBOs();

        void processPBO(int x, int y);
        void processPBOResults();

        std::set<TileCoord> dirtyTiles;
        void checkAndSaveTiles(float startX, float startY, float endX, float endY);

        std::unique_ptr<UndoSystem> undoSystem;

        struct PboRequest {
            int tileX, tileY;
            int stepID;
        };
        PboRequest pboRequests[16];
};
