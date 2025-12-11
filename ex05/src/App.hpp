#pragma once
#include <memory>
#include "Window.hpp"
#include "InputManager.hpp"
#include "Canvas.hpp"
#include "Renderer.hpp"
#include "Brush.hpp"
#include "UndoSystem.hpp"

class App {
public:
    App(int width, int height, const char* title, float canvasSize);
    ~App() = default;

    void run();
    void saveImage(const char* filename);

private:
    std::unique_ptr<Window> window;
    std::unique_ptr<InputManager> inputManager;
    std::unique_ptr<Canvas> canvas;
    std::unique_ptr<Renderer> renderer;
    std::unique_ptr<Brush> brush;
    std::unique_ptr<UndoSystem> undoSystem;

    float canvasSize;
    const int tileSize = 128;

    // 描画状態
    bool isDrawing = false;
    bool wasDrawing = false;
    bool isEraser = false;
    float lastX = 0.0f;
    float lastY = 0.0f;

    void processInput(int width, int height, float scaleX, float scaleY);
    void handleKeyboardInput();
    void handleMouseInput(int width, int height, float scaleX, float scaleY);
    void render(int width, int height, float scaleX, float scaleY);
};
