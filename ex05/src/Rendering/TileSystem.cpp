#include "TileSystem.hpp"
#include "History/HistoryManager.hpp"
#include <algorithm>
#include <iostream>

TileSystem::TileSystem(int canvasSize, int tileSize)
    : canvasSize(canvasSize), tileSize(tileSize) {
    initPBOs();
}

TileSystem::~TileSystem() {
    glDeleteBuffers(PBO_COUNT, pboIds);
}

void TileSystem::initPBOs() {
    glGenBuffers(PBO_COUNT, pboIds);
    for (int i = 0; i < PBO_COUNT; i++) {
        glBindBuffer(GL_PIXEL_PACK_BUFFER, pboIds[i]);
        glBufferData(GL_PIXEL_PACK_BUFFER, tileSize * tileSize * channels * sizeof(uint8_t), nullptr, GL_STREAM_READ);
    }
    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
}

void TileSystem::markDirtyTiles(float startX, float startY, float endX, float endY, float brushRadius) {
    float radius = brushRadius / 2.0f + 2.0f;

    float minX = std::min(startX, endX) - radius;
    float maxX = std::max(startX, endX) + radius;
    float minY = std::min(startY, endY) - radius;
    float maxY = std::max(startY, endY) + radius;

    int tileStartX = static_cast<int>(minX) / tileSize;
    int tileEndX = static_cast<int>(maxX) / tileSize;
    int tileStartY = static_cast<int>(minY) / tileSize;
    int tileEndY = static_cast<int>(maxY) / tileSize;

    int tileMaxIndex = canvasSize / tileSize - 1;
    tileStartX = std::max(0, std::min(tileStartX, tileMaxIndex));
    tileEndX = std::max(0, std::min(tileEndX, tileMaxIndex));
    tileStartY = std::max(0, std::min(tileStartY, tileMaxIndex));
    tileEndY = std::max(0, std::min(tileEndY, tileMaxIndex));

    for (int ty = tileStartY; ty <= tileEndY; ++ty) {
        for (int tx = tileStartX; tx <= tileEndX; ++tx) {
            TileCoord coord = {tx, ty};
            if (dirtyTiles.find(coord) == dirtyTiles.end()) {
                dirtyTiles.insert(coord);
                pendingNewTiles.push_back(coord);
            }
        }
    }
}

void TileSystem::clearDirtyTiles() {
    dirtyTiles.clear();
    pendingNewTiles.clear();
}

void TileSystem::capturePendingTiles(int stepID) {
    for (const auto& coord : pendingNewTiles) {
        beginTileCapture(coord.x * tileSize, coord.y * tileSize, stepID);
    }
    pendingNewTiles.clear();
}

void TileSystem::beginTileCapture(int pixelX, int pixelY, int stepID) {
    if (pendingPBOs >= PBO_COUNT) {
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

void TileSystem::processPendingCaptures(HistoryManager& historyManager) {
    while (pendingPBOs > 0) {
        glBindBuffer(GL_PIXEL_PACK_BUFFER, pboIds[pboTail]);

        GLubyte* ptr = static_cast<GLubyte*>(glMapBufferRange(GL_PIXEL_PACK_BUFFER, 0, tileSize * tileSize * channels, GL_MAP_READ_BIT));
        if (ptr) {
            PboRequest& req = pboRequests[pboTail];
            historyManager.pushBeforeTile(req.tileX, req.tileY, req.stepID, ptr);

            glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
            pboTail = (pboTail + 1) % PBO_COUNT;
            pendingPBOs--;
        } else {
            break;
        }
    }

    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
}

void TileSystem::saveAfterTiles(HistoryManager& historyManager) {
    std::vector<uint8_t> tilePixels(tileSize * tileSize * 4);
    int currentStepID = historyManager.getCurrentStepID();

    for (const auto& coord : dirtyTiles) {
        int pixelX = coord.x * tileSize;
        int pixelY = coord.y * tileSize;

        glReadPixels(pixelX, pixelY, tileSize, tileSize, GL_RGBA, GL_UNSIGNED_BYTE, tilePixels.data());
        historyManager.pushAfterTile(pixelX, pixelY, currentStepID, tilePixels.data());
    }
}
