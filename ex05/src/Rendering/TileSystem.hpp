#pragma once
#include <GL/glew.h>
#include <set>
#include <vector>
#include <cstdint>
#include "History/HistoryTypes.hpp"

class HistoryManager;

struct TileCoord {
    int x, y;

    bool operator<(const TileCoord& other) const {
        if (x != other.x) {
            return x < other.x;
        }
        return y < other.y;
    }
};

// タイルシステム: ダーティタイル追跡とPBO非同期転送を管理
class TileSystem {
public:
    TileSystem(int canvasSize, int tileSize);
    ~TileSystem();

    // コピー禁止
    TileSystem(const TileSystem&) = delete;
    TileSystem& operator=(const TileSystem&) = delete;

    int getTileSize() const { return tileSize; }
    int getTileCount() const { return canvasSize / tileSize; }

    // ダーティタイル管理
    void markDirtyTiles(float startX, float startY, float endX, float endY, float brushRadius);
    void clearDirtyTiles();
    bool hasDirtyTiles() const { return !dirtyTiles.empty(); }
    const std::set<TileCoord>& getDirtyTiles() const { return dirtyTiles; }

    // PBO非同期転送(描画前タイルキャプチャ)
    void capturePendingTiles(int stepID);
    void processPendingCaptures(HistoryManager& historyManager);

    // 描画後タイル保存
    void saveAfterTiles(HistoryManager& historyManager);

private:
    int canvasSize;
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
    std::vector<TileCoord> pendingNewTiles;

    void initPBOs();
    void beginTileCapture(int pixelX, int pixelY, int stepID);
};
