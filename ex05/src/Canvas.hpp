#pragma once
#include <GLES3/gl32.h>
#include <memory>
#include <set>
#include <vector>
#include "FrameBuffer.hpp"
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

class Canvas {
public:
    Canvas(int size, int tileSize);
    ~Canvas();

    void bind();
    void unbind();

    GLuint getTexture() const;
    int getSize() const { return size; }
    int getTileSize() const { return tileSize; }

    // タイルシステム
    void markDirtyTiles(float startX, float startY, float endX, float endY, float brushRadius);
    void clearDirtyTiles();
    bool hasDirtyTiles() const { return !dirtyTiles.empty(); }

    // PBO非同期転送
    void capturePendingTiles(int stepID);
    void processPendingCaptures(UndoSystem& undoSystem);
    void saveAfterTiles(UndoSystem& undoSystem);

    // Undo/Redoタイル復元
    void restoreTiles(const std::vector<TileData>& tiles);

    // 画像保存
    std::vector<uint8_t> readPixels() const;

private:
    std::unique_ptr<FrameBuffer> fbo;
    int size;
    int tileSize;
    static constexpr int channels = 4;

    // PBO管理
    static constexpr int PBO_COUNT = 128;
    GLuint pboIds[PBO_COUNT];
    int pboHead = 0;
    int pboTail = 0;
    int pendingPBOs = 0;

    struct PboRequest {
        int tileX, tileY;
        int stepID;
    };
    PboRequest pboRequests[PBO_COUNT];

    // ダーティタイル管理
    std::set<TileCoord> dirtyTiles;
    std::vector<TileCoord> pendingNewTiles; // PBOキャプチャ待ちの新規タイル

    void initPBOs();
    void beginTileCapture(int pixelX, int pixelY, int stepID);
};
