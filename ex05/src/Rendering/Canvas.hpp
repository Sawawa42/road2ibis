#pragma once
#include <GLES3/gl32.h>
#include <memory>
#include <vector>
#include "Graphics/FrameBuffer.hpp"
#include "LayerTexture.hpp"
#include "TileSystem.hpp"
#include "History/HistoryTypes.hpp"

class HistoryManager;

// Canvas: レイヤーテクスチャとタイルシステムを統合管理するファサード
class Canvas {
public:
    Canvas(int size, int tileSize);
    ~Canvas() = default;

    // 描画先の切り替え
    void bind();
    void unbind();

    // レイヤーテクスチャアクセス
    GLuint getTexture() const;
    int getSize() const { return size; }
    int getTileSize() const { return tileSystem->getTileSize(); }

    // タイルシステムへの委譲
    void markDirtyTiles(float startX, float startY, float endX, float endY, float brushRadius);
    void clearDirtyTiles();
    bool hasDirtyTiles() const;

    // PBO非同期転送
    void capturePendingTiles(int stepID);
    void processPendingCaptures(HistoryManager& historyManager);
    void saveAfterTiles(HistoryManager& historyManager);

    // Undo/Redoタイル復元
    void restoreTiles(const std::vector<TileData>& tiles);

    // 画像保存
    std::vector<uint8_t> readPixels() const;

private:
    int size;
    std::unique_ptr<LayerTexture> layerTexture;
    std::unique_ptr<FrameBuffer> fbo;
    std::unique_ptr<TileSystem> tileSystem;
};
