#include "Canvas.hpp"
#include "History/HistoryManager.hpp"

Canvas::Canvas(int size, int tileSize)
    : size(size) {
    // レイヤーテクスチャを作成
    layerTexture = std::make_unique<LayerTexture>(size, size);

    // FBOを作成し、レイヤーテクスチャをアタッチ
    fbo = std::make_unique<FrameBuffer>(layerTexture->getId());

    // タイルシステムを初期化
    tileSystem = std::make_unique<TileSystem>(size, tileSize);

    // キャンバスを白(透明)で初期化
    layerTexture->clear(1.0f, 1.0f, 1.0f, 0.0f);
}

void Canvas::bind() {
    fbo->bind();
    glViewport(0, 0, size, size);
}

void Canvas::unbind() {
    fbo->unbind();
}

GLuint Canvas::getTexture() const {
    return layerTexture->getId();
}

void Canvas::markDirtyTiles(float startX, float startY, float endX, float endY, float brushRadius) {
    tileSystem->markDirtyTiles(startX, startY, endX, endY, brushRadius);
}

void Canvas::clearDirtyTiles() {
    tileSystem->clearDirtyTiles();
}

bool Canvas::hasDirtyTiles() const {
    return tileSystem->hasDirtyTiles();
}

void Canvas::capturePendingTiles(int stepID) {
    tileSystem->capturePendingTiles(stepID);
}

void Canvas::processPendingCaptures(HistoryManager& historyManager) {
    tileSystem->processPendingCaptures(historyManager);
}

void Canvas::saveAfterTiles(HistoryManager& historyManager) {
    bind();
    tileSystem->saveAfterTiles(historyManager);
    unbind();
}

void Canvas::restoreTiles(const std::vector<TileData>& tiles) {
    int tileSize = tileSystem->getTileSize();
    for (const auto& tile : tiles) {
        layerTexture->updateTile(tile.tileX, tile.tileY, tileSize, tileSize, tile.pixels.data());
    }
}

std::vector<uint8_t> Canvas::readPixels() const {
    return layerTexture->readAllPixels();
}
