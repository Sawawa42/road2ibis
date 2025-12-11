#pragma once
#include <string>
#include <vector>
#include <map>
#include <mutex>
#include "HistoryTypes.hpp"

// 永続化層: ファイルI/Oを担当
class HistoryStorage {
public:
    explicit HistoryStorage(const std::string& filename, int tileSize);
    ~HistoryStorage() = default;

    // タイルデータの書き込み
    TileRecord writeTile(const TileData& data);

    // タイルデータの読み込み
    TileData readTile(const TileRecord& record) const;
    std::vector<TileData> readTiles(const std::vector<TileRecord>& records) const;

    // ファイル操作
    void truncate(size_t offset);
    void clear();

    // 現在のファイルオフセット
    size_t getCurrentOffset() const { return currentOffset; }
    void setCurrentOffset(size_t offset) { currentOffset = offset; }

private:
    std::string filename;
    int tileSize;
    size_t currentOffset = 0;
    mutable std::mutex fileMutex;

    bool isTileEmpty(const std::vector<uint8_t>& pixels) const;
};
