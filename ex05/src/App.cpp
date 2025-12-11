#include "App_new.hpp"
#include "../external/lodepng/lodepng.h"
#include <iostream>
#include <vector>
#include <cstring>

App::App(int width, int height, const char* title, float canvasSize)
    : canvasSize(canvasSize) {
    window = std::make_unique<Window>(width, height, title);
    inputManager = std::make_unique<InputManager>(window->getHandle());
    canvas = std::make_unique<Canvas>(static_cast<int>(canvasSize), tileSize);
    renderer = std::make_unique<Renderer>();
    brush = std::make_unique<Brush>();
    undoSystem = std::make_unique<UndoSystem>("history.bin", tileSize);
}

void App::run() {
    while (!window->shouldClose()) {
        int width, height;
        window->getFramebufferSize(width, height);

        float scaleX = 1.0f;
        float scaleY = 1.0f;

        if (width > height) {
            scaleX = static_cast<float>(height) / static_cast<float>(width);
        } else {
            scaleY = static_cast<float>(width) / static_cast<float>(height);
        }

        inputManager->update();
        processInput(width, height, scaleX, scaleY);
        render(width, height, scaleX, scaleY);
        canvas->processPendingCaptures(*undoSystem);

        window->swapBuffers();
        window->pollEvents();
    }
}

void App::processInput(int width, int height, float scaleX, float scaleY) {
    handleKeyboardInput();
    handleMouseInput(width, height, scaleX, scaleY);
}

void App::handleKeyboardInput() {
    InputAction action = inputManager->getTriggeredAction();

    switch (action) {
        case InputAction::SetEraser:
            brush->setColor(0, 0, 0, 0);
            brush->setSize(30.0f);
            isEraser = true;
            break;
        case InputAction::SetBrushBlack:
            brush->setColor(0, 0, 0, 255);
            brush->setSize(30.0f);
            isEraser = false;
            break;
        case InputAction::SetBrushRed:
            brush->setColor(255, 0, 0, 255);
            brush->setSize(30.0f);
            isEraser = false;
            break;
        case InputAction::SetBrushGreen:
            brush->setColor(0, 255, 0, 255);
            brush->setSize(30.0f);
            isEraser = false;
            break;
        case InputAction::SetBrushBlue:
            brush->setColor(0, 0, 255, 255);
            brush->setSize(30.0f);
            isEraser = false;
            break;
        case InputAction::Save:
            saveImage("output.png");
            break;
        case InputAction::Undo: {
            static bool ctrlzPressed = false;
            if (!ctrlzPressed) {
                std::cout << "Undo requested" << std::endl;
                std::vector<TileData> restore = undoSystem->undo();
                if (!restore.empty()) {
                    canvas->restoreTiles(restore);
                    std::cout << "Undo performed: stepID=" << undoSystem->getCurrentStepID() << std::endl;
                }
                ctrlzPressed = true;
            }
            break;
        }
        case InputAction::Redo: {
            static bool ctrlyPressed = false;
            if (!ctrlyPressed) {
                std::cout << "Redo requested" << std::endl;
                std::vector<TileData> restore = undoSystem->redo();
                if (!restore.empty()) {
                    canvas->restoreTiles(restore);
                    std::cout << "Redo performed: stepID=" << undoSystem->getCurrentStepID() << std::endl;
                }
                ctrlyPressed = true;
            }
            break;
        }
        default:
            break;
    }
}

void App::handleMouseInput(int width, int height, float scaleX, float scaleY) {
    const MouseState& mouse = inputManager->getMouseState();

    if (mouse.leftPressed) {
        if (mouse.leftJustPressed) {
            undoSystem->incrementStepID();
            canvas->clearDirtyTiles();
        }

        // 座標変換
        float ndcX = (static_cast<float>(mouse.x) / width) * 2.0f - 1.0f;
        float ndcY = 1.0f - (static_cast<float>(mouse.y) / height) * 2.0f;
        float canvasX = ndcX / scaleX;
        float canvasY = ndcY / scaleY;

        bool inside = (canvasX >= -1.0f && canvasX <= 1.0f && canvasY >= -1.0f && canvasY <= 1.0f);
        if (inside) {
            canvas->bind();
            glViewport(0, 0, static_cast<int>(canvasSize), static_cast<int>(canvasSize));

            if (isEraser) {
                glBlendFunc(GL_ONE, GL_ZERO);
            } else {
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            }

            float pxCurrentX = (canvasX + 1.0f) / 2.0f * canvasSize;
            float pxCurrentY = (canvasY + 1.0f) / 2.0f * canvasSize;
            float pxLastX = (lastX + 1.0f) / 2.0f * canvasSize;
            float pxLastY = (lastY + 1.0f) / 2.0f * canvasSize;

            if (!isDrawing) {
                pxLastX = pxCurrentX;
                pxLastY = pxCurrentY;
            }

            // ダーティタイルをマーク
            float brushRadius = brush->getSize();
            canvas->markDirtyTiles(
                isDrawing ? pxLastX : pxCurrentX,
                isDrawing ? pxLastY : pxCurrentY,
                pxCurrentX, pxCurrentY, brushRadius);

            // ダーティタイルのPBOキャプチャを開始
            canvas->capturePendingTiles(undoSystem->getCurrentStepID());

            brush->begin();
            if (isDrawing) {
                brush->drawLine(lastX, lastY, canvasX, canvasY, canvasSize);
            } else {
                brush->drawLine(canvasX, canvasY, canvasX, canvasY, canvasSize);
            }

            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            canvas->unbind();
        }

        lastX = canvasX;
        lastY = canvasY;
        isDrawing = true;
        wasDrawing = true;
    } else {
        if (wasDrawing && canvas->hasDirtyTiles()) {
            canvas->saveAfterTiles(*undoSystem);
        }
        isDrawing = false;
        wasDrawing = false;
    }
}

void App::render(int width, int height, float scaleX, float scaleY) {
    renderer->setViewport(width, height);
    renderer->clear(200, 200, 200, 255);
    renderer->renderCanvas(canvas->getTexture(), scaleX, scaleY);
}

void App::saveImage(const char* filename) {
    std::vector<uint8_t> pixels = canvas->readPixels();

    // 画像を上下反転
    int size = static_cast<int>(canvasSize);
    std::vector<uint8_t> flippedPixels(size * size * 4);
    for (int y = 0; y < size; ++y) {
        memcpy(&flippedPixels[y * size * 4],
               &pixels[(size - 1 - y) * size * 4],
               size * 4);
    }

    unsigned error = lodepng::encode(filename, flippedPixels, static_cast<unsigned>(size), static_cast<unsigned>(size));
    if (error) {
        std::cerr << "Error saving image: " << lodepng_error_text(error) << std::endl;
    }
}
