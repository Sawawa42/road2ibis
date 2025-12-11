#pragma once
#include <vector>
#include <cstdint>

// タイルデータ
struct TileData {
    int tileX, tileY;
    int stepID;
    std::vector<uint8_t> pixels;
};

// ファイル内のタイル記録情報
struct TileRecord {
    int tileX, tileY;
    uint8_t type;    // TYPE_EMPTY or TYPE_RAW
    size_t offset;   // ファイル内のオフセット
    size_t size;     // ピクセルデータのサイズ
};

// タイルタイプ定数
constexpr uint8_t TILE_TYPE_EMPTY = 0;
constexpr uint8_t TILE_TYPE_RAW = 1;
