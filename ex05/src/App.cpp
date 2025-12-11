#include "App.hpp"
#include "../external/lodepng/lodepng.h"
#include <iostream>
#include <vector>

const char* vertexShaderSource = R"(#version 300 es
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTexCoord;
out vec2 TexCoord;
uniform vec2 uScale;
void main() {
    gl_Position = vec4(aPos.x * uScale.x, aPos.y * uScale.y, aPos.z, 1.0);
    TexCoord = aTexCoord;
})";

const char* fragmentShaderSource = R"(#version 300 es
precision mediump float;
in vec2 TexCoord;
uniform sampler2D uTexture;
out vec4 FragColor;
void main() {
    vec4 texColor = texture(uTexture, TexCoord);

    vec3 bgColor = vec3(1.0, 1.0, 1.0); // 白背景
    vec3 blendedColor = mix(bgColor, texColor.rgb, texColor.a);

    FragColor = vec4(blendedColor, 1.0);
})";

App::App(int width, int height, const char* title, float size)
    : isDrawing(false), lastX(0.0f), lastY(0.0f), fboSize(size) {
    // GLFW初期化
    if (!glfwInit()) {
        exit(-1);
    }
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);

    window = glfwCreateWindow(width, height, title, nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        exit(-1);
    }
    glfwMakeContextCurrent(window);

    initOpenGL();
}

App::~App() {
    glfwDestroyWindow(window);
    glfwTerminate();
}

void App::initOpenGL() {
    // 透過処理を有効化
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    shader = std::make_unique<Shader>(vertexShaderSource, fragmentShaderSource);

    std::vector<float> vertices = {
        -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, // 左上
        -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, // 左下
        1.0f, 1.0f, 0.0f, 1.0f, 1.0f,  // 右上
        1.0f, -1.0f, 0.0f, 1.0f, 0.0f, // 右下
    };
    mesh = std::make_unique<Mesh>(vertices, MeshFormat::XYZ_UV);

    canvas = std::make_unique<FrameBuffer>((int)fboSize, (int)fboSize);
    brush = std::make_unique<Brush>();

    canvas->bind();
    glViewport(0, 0, (int)fboSize, (int)fboSize);
    clearColorUint8(255, 255, 255, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    canvas->unbind();

    initPBOs();

    undoSystem = std::make_unique<UndoSystem>("history.bin", tileSize);
}

void App::run() {
    while (!glfwWindowShouldClose(window)) {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        float scaleX = 1.0f;
        float scaleY = 1.0f;

        if (width > height) {
            scaleX = (float)height / (float)width;
        } else {
            scaleY = (float)width / (float)height;
        }

        processInput(width, height, scaleX, scaleY);

        render(width, height, scaleX, scaleY);

        processPBOResults();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}

void App::processInput(int width, int height, float scaleX, float scaleY) {
    static bool isEraser = false;

    if (glfwGetKey(window, GLFW_KEY_0) == GLFW_PRESS) {
        // 30pxの透明ブラシ(消しゴム代わり)
        brush->setColor(0, 0, 0, 0);
        brush->setSize(30.0f);
        isEraser = true;
    } else if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) {
        // 30pxの黒色ブラシ
        brush->setColor(0, 0, 0, 255);
        brush->setSize(30.0f);
        isEraser = false;
    } else if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
        // 30pxの赤色ブラシ
        brush->setColor(255, 0, 0, 255);
        brush->setSize(30.0f);
        isEraser = false;
    } else if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS) {
        // 30pxの緑色ブラシ
        brush->setColor(0, 255, 0, 255);
        brush->setSize(30.0f);
        isEraser = false;
    } else if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS) {
        // 30pxの青色ブラシ
        brush->setColor(0, 0, 255, 255);
        brush->setSize(30.0f);
        isEraser = false;
    } else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        saveImage("output.png");
    }
    static bool zPressed = false;
    if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS) {
        if (!zPressed) {
            std::cout << "Undo requested" << std::endl;
            std::vector<TileData> restore = undoSystem->undo();
            if (!restore.empty()) {
                canvas->bind();
                for (const auto& tile: restore) {
                    glTexSubImage2D(GL_TEXTURE_2D, 0,
                                    tile.tileX * tileSize,
                                    tile.tileY * tileSize,
                                    tileSize, tileSize,
                                    GL_RGBA, GL_UNSIGNED_BYTE,
                                    tile.pixels.data());
                }

                canvas->unbind();
                std::cout << "Undo performed: stepID=" << undoSystem->getCurrentStepID() << std::endl;
            }
            zPressed = true;
        }
    } else {
        zPressed = false;
    }
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
        undoSystem->incrementStepID();

        double mouseX, mouseY;
        glfwGetCursorPos(window, &mouseX, &mouseY);

        // 1. ウィンドウ座標 -> NDC (-1 ~ 1)
        // Y軸は反転させる
        float ndcX = ((float)mouseX / width) * 2.0f - 1.0f;
        float ndcY = 1.0f - ((float)mouseY / height) * 2.0f;

        // 2. スケール逆補正 -> キャンバス上のNDC
        float canvasX = ndcX / scaleX;
        float canvasY = ndcY / scaleY;

        // 3. キャンバス内か判定
        bool inside = (canvasX >= -1.0f && canvasX <= 1.0f && canvasY >= -1.0f && canvasY <= 1.0f);
        if (inside) {
            canvas->bind();
            glViewport(0, 0, (int)fboSize, (int)fboSize);

            if (isEraser) {
                glBlendFunc(GL_ONE, GL_ZERO);
            } else {
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            }

            float pxCurrentX = (canvasX + 1.0f) / 2.0f * fboSize;
            float pxCurrentY = (canvasY + 1.0f) / 2.0f * fboSize;
            float pxLastX = (lastX + 1.0f) / 2.0f * fboSize;
            float pxLastY = (lastY + 1.0f) / 2.0f * fboSize;

            if (!isDrawing) {
                dirtyTiles.clear();
                pxLastX = pxCurrentX;
                pxLastY = pxCurrentY;
            }

            if (isDrawing) {
                checkAndSaveTiles(pxLastX, pxLastY, pxCurrentX, pxCurrentY);
            } else {
                checkAndSaveTiles(pxCurrentX, pxCurrentY, pxCurrentX, pxCurrentY);
            }

            brush->begin();
            if (isDrawing) {
                brush->drawLine(lastX, lastY, canvasX, canvasY, fboSize);
            } else {
                brush->drawLine(canvasX, canvasY, canvasX, canvasY, fboSize);
                isDrawing = true;
            }

            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            canvas->unbind();
        }
        lastX = canvasX;
        lastY = canvasY;
        isDrawing = true;
    } else {
        isDrawing = false;
    }
}

void App::render(int width, int height, float scaleX, float scaleY) {
    glViewport(0, 0, width, height);
    clearColorUint8(200, 200, 200, 255);
    glClear(GL_COLOR_BUFFER_BIT);

    shader->use();
    glUniform2f(glGetUniformLocation(shader->ID, "uScale"), scaleX, scaleY);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, canvas->getTexture());
    glUniform1i(glGetUniformLocation(shader->ID, "uTexture"), 0);

    mesh->draw(GL_TRIANGLE_STRIP);
}

void App::clearColorUint8(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    glClearColor(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);
}

void App::saveImage(const char* filename) {
    canvas->bind();
    std::vector<uint8_t> pixels((size_t)(fboSize * fboSize * 4));
    glReadPixels(0, 0, (int)fboSize, (int)fboSize, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
    canvas->unbind();

    // 画像を上下反転
    std::vector<uint8_t> flippedPixels((size_t)(fboSize * fboSize * 4));
    for (int y = 0; y < (int)fboSize; ++y) {
        memcpy(&flippedPixels[y * (int)fboSize * 4],
               &pixels[((int)fboSize - 1 - y) * (int)fboSize * 4],
               (size_t)((int)fboSize * 4));
    }

    unsigned error = lodepng::encode(filename, flippedPixels, (unsigned)fboSize, (unsigned)fboSize);
    if (error) {
        std::cerr << "Error saving image: " << lodepng_error_text(error) << std::endl;
    }
}

void App::initPBOs() {
    glGenBuffers(16, pboIds); // これはSeparateという格納方式だが、Interleavedのほうがキャッシュ的にいいらしい？
    for (int i = 0; i < 16; i++) {
        glBindBuffer(GL_PIXEL_PACK_BUFFER, pboIds[i]);
        // GL_STREAM_READ: GPUが書き込み、CPUが読み取りする
        glBufferData(GL_PIXEL_PACK_BUFFER, tileSize * tileSize * channels * sizeof(uint8_t), nullptr, GL_STREAM_READ);
    }

    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
}

void App::processPBO(int x, int y) {
    if (pendingPBOs >= 16) {
        // 処理待ちPBOが最大数に達している場合、結果を取得してから進める
        processPBOResults();
    }

    pboRequests[pboHead].tileX = x;
    pboRequests[pboHead].tileY = y;
    pboRequests[pboHead].stepID = undoSystem->getCurrentStepID();

    glBindBuffer(GL_PIXEL_PACK_BUFFER, pboIds[pboHead]);
    glReadPixels(x, y, tileSize, tileSize, GL_RGBA, GL_UNSIGNED_BYTE, 0);
    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

    // PBOのインデックスを進める
    pboHead = (pboHead + 1) % 16;
    pendingPBOs++;
}

void App::processPBOResults() {
    if (pendingPBOs == 0) {
        return; // 処理待ちPBOがない
    }

    // glClientWaitSync (Fence)を使う方法もある？
    glBindBuffer(GL_PIXEL_PACK_BUFFER, pboIds[pboTail]);

    GLubyte* ptr = (GLubyte*)glMapBufferRange(GL_PIXEL_PACK_BUFFER, 0, tileSize * tileSize * channels, GL_MAP_READ_BIT);
    if (ptr) {
        PboRequest& req = pboRequests[pboTail];
        undoSystem->pushTile(req.tileX, req.tileY, req.stepID, ptr);

        glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
        pboTail = (pboTail + 1) % 16;
        pendingPBOs--;
    }

    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
}

void App::checkAndSaveTiles(float startX, float startY, float endX, float endY) {
    // ブラシの移動範囲を含む影響範囲(バウンディングボックス)を計算
    float radius = 30.0f / 2.0f + 2.0f; // ブラシ半径を少し余裕を持たせて計算(ハードコードなので要改善)

    float minX = std::min(startX, endX) - radius;
    float maxX = std::max(startX, endX) + radius;
    float minY = std::min(startY, endY) - radius;
    float maxY = std::max(startY, endY) + radius;

    int tileStartX = (int)(minX) / tileSize;
    int tileEndX = (int)(maxX) / tileSize;
    int tileStartY = (int)(minY) / tileSize;
    int tileEndY = (int)(maxY) / tileSize;

    int tileMaxIndex = (int)(fboSize) / tileSize - 1;
    tileStartX = std::max(0, std::min(tileStartX, tileMaxIndex));
    tileEndX = std::max(0, std::min(tileEndX, tileMaxIndex));
    tileStartY = std::max(0, std::min(tileStartY, tileMaxIndex));
    tileEndY = std::max(0, std::min(tileEndY, tileMaxIndex));

    for (int ty = tileStartY; ty <= tileEndY; ++ty) {
        for (int tx = tileStartX; tx <= tileEndX; ++tx) {
            TileCoord coord = {tx, ty};
            if (dirtyTiles.find(coord) == dirtyTiles.end()) {
                processPBO(tx * tileSize, ty * tileSize);
                dirtyTiles.insert(coord);

                std::cout << "debug: [" << tx << ", " << ty << "]" << std::endl;
            }
        }
    }
}
