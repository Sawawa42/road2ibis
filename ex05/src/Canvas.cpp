#include "Canvas.hpp"
#include <algorithm>
#include <iostream>

Canvas::Canvas(int size, int tileSize)
    : size(size), tileSize(tileSize) {
    fbo = std::make_unique<FrameBuffer>(size, size);

    // キャンバスを透明で初期化
    bind();
    glViewport(0, 0, size, size);
    glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    unbind();

    initPBOs();
}

Canvas::~Canvas() {
    glDeleteBuffers(PBO_COUNT, pboIds);
}

void Canvas::bind() {
    fbo->bind();
}

void Canvas::unbind() {
    fbo->unbind();
}

GLuint Canvas::getTexture() const {
    return fbo->getTexture();
}

void Canvas::initPBOs() {
    glGenBuffers(PBO_COUNT, pboIds);
    for (int i = 0; i < PBO_COUNT; i++) {
        glBindBuffer(GL_PIXEL_PACK_BUFFER, pboIds[i]);
        glBufferData(GL_PIXEL_PACK_BUFFER, tileSize * tileSize * channels * sizeof(uint8_t), nullptr, GL_STREAM_READ);
    }
    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
}

void Canvas::markDirtyTiles(float startX, float startY, float endX, float endY, float brushRadius) {
    float radius = brushRadius / 2.0f + 2.0f;

    float minX = std::min(startX, endX) - radius;
    float maxX = std::max(startX, endX) + radius;
    float minY = std::min(startY, endY) - radius;
    float maxY = std::max(startY, endY) + radius;

    int tileStartX = static_cast<int>(minX) / tileSize;
    int tileEndX = static_cast<int>(maxX) / tileSize;
    int tileStartY = static_cast<int>(minY) / tileSize;
    int tileEndY = static_cast<int>(maxY) / tileSize;

    int tileMaxIndex = size / tileSize - 1;
    tileStartX = std::max(0, std::min(tileStartX, tileMaxIndex));
    tileEndX = std::max(0, std::min(tileEndX, tileMaxIndex));
    tileStartY = std::max(0, std::min(tileStartY, tileMaxIndex));
    tileEndY = std::max(0, std::min(tileEndY, tileMaxIndex));

    for (int ty = tileStartY; ty <= tileEndY; ++ty) {
        for (int tx = tileStartX; tx <= tileEndX; ++tx) {
            TileCoord coord = {tx, ty};
            if (dirtyTiles.find(coord) == dirtyTiles.end()) {
                dirtyTiles.insert(coord);
                // 新しいダーティタイルのPBOキャプチャを追加
                pendingNewTiles.push_back(coord);
            }
        }
    }
}

void Canvas::clearDirtyTiles() {
    dirtyTiles.clear();
    pendingNewTiles.clear();
}

void Canvas::capturePendingTiles(int stepID) {
    if (pendingPBOs >= PBO_COUNT - 2) {
        std::cerr << "Warning: PBO buffer nearly full (" << pendingPBOs << "/" << PBO_COUNT << ")" << std::endl;
    }
    
    for (const auto& coord : pendingNewTiles) {
        if (pendingPBOs >= PBO_COUNT) {
            std::cerr << "Error: Skipping tile capture due to PBO buffer overflow" << std::endl;
            break;
        }
        beginTileCapture(coord.x * tileSize, coord.y * tileSize, stepID);
    }
    pendingNewTiles.clear();
}

void Canvas::beginTileCapture(int pixelX, int pixelY, int stepID) {
    if (pendingPBOs >= PBO_COUNT) {
        std::cerr << "Warning: PBO buffer full, processing pending captures" << std::endl;
        // バッファが満杯の場合、強制的に処理を進める
        // これを呼ぶにはUndoSystemの参照が必要なため、一旦スキップするしかない
        // 代わりに最も古いPBOを破棄する
        std::cerr << "Error: Cannot capture tile - PBO buffer overflow" << std::endl;
        return;
    }

    pboRequests[pboHead].tileX = pixelX;
    pboRequests[pboHead].tileY = pixelY;
    pboRequests[pboHead].stepID = stepID;

    glBindBuffer(GL_PIXEL_PACK_BUFFER, pboIds[pboHead]);
    glReadPixels(pixelX, pixelY, tileSize, tileSize, GL_RGBA, GL_UNSIGNED_BYTE, 0);
    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

    pboHead = (pboHead + 1) % PBO_COUNT;
    pendingPBOs++;
}

void Canvas::processPendingCaptures(UndoSystem& undoSystem) {
    // 全ての保留中のPBOを処理（1フレームで1つだけでなく、可能な限り処理）
    while (pendingPBOs > 0) {
        glBindBuffer(GL_PIXEL_PACK_BUFFER, pboIds[pboTail]);

        GLubyte* ptr = static_cast<GLubyte*>(glMapBufferRange(GL_PIXEL_PACK_BUFFER, 0, tileSize * tileSize * channels, GL_MAP_READ_BIT));
        if (ptr) {
            PboRequest& req = pboRequests[pboTail];
            undoSystem.pushTile(req.tileX, req.tileY, req.stepID, ptr);

            glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
            pboTail = (pboTail + 1) % PBO_COUNT;
            pendingPBOs--;
        } else {
            // マップできない場合は次のフレームで再試行
            break;
        }
    }

    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
}

void Canvas::saveAfterTiles(UndoSystem& undoSystem) {
    bind();

    std::vector<uint8_t> tilePixels(tileSize * tileSize * 4);
    int currentStepID = undoSystem.getCurrentStepID();

    for (const auto& coord : dirtyTiles) {
        int pixelX = coord.x * tileSize;
        int pixelY = coord.y * tileSize;

        glReadPixels(pixelX, pixelY, tileSize, tileSize, GL_RGBA, GL_UNSIGNED_BYTE, tilePixels.data());
        undoSystem.pushAfterTile(pixelX, pixelY, currentStepID, tilePixels.data());
    }

    unbind();
}

void Canvas::restoreTiles(const std::vector<TileData>& tiles) {
    bind();
    for (const auto& tile : tiles) {
        glTexSubImage2D(GL_TEXTURE_2D, 0,
                        tile.tileX,
                        tile.tileY,
                        tileSize, tileSize,
                        GL_RGBA, GL_UNSIGNED_BYTE,
                        tile.pixels.data());
    }
    unbind();
}

std::vector<uint8_t> Canvas::readPixels() const {
    fbo->bind();
    std::vector<uint8_t> pixels(size * size * 4);
    glReadPixels(0, 0, size, size, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
    fbo->unbind();
    return pixels;
}
