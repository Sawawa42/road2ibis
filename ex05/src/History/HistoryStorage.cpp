#include "HistoryStorage.hpp"
#include <fstream>
#include <iostream>
#include <unistd.h>

HistoryStorage::HistoryStorage(const std::string& filename, int tileSize)
    : filename(filename), tileSize(tileSize), currentOffset(0) {
    // ファイルを初期化（空にする）
    clear();
}

void HistoryStorage::clear() {
    std::lock_guard<std::mutex> lock(fileMutex);
    std::ofstream ofs(filename, std::ios::binary | std::ios::trunc);
    ofs.close();
    currentOffset = 0;
}

bool HistoryStorage::isTileEmpty(const std::vector<uint8_t>& pixels) const {
    size_t pixelCount = pixels.size() / 4;
    for (size_t i = 0; i < pixelCount; ++i) {
        if (pixels[i * 4 + 3] != 0) {  // Alphaチャネルが0でない
            return false;
        }
    }
    return true;
}

TileRecord HistoryStorage::writeTile(const TileData& data) {
    std::lock_guard<std::mutex> lock(fileMutex);

    std::ofstream ofs(filename, std::ios::binary | std::ios::app);
    if (!ofs.is_open()) {
        std::cerr << "Failed to open history file for writing" << std::endl;
        return TileRecord{};
    }

    size_t startOffset = currentOffset;

    // ヘッダー書き込み: stepID, tileX, tileY (各4バイト = 12バイト)
    ofs.write(reinterpret_cast<const char*>(&data.stepID), sizeof(int));
    ofs.write(reinterpret_cast<const char*>(&data.tileX), sizeof(int));
    ofs.write(reinterpret_cast<const char*>(&data.tileY), sizeof(int));
    currentOffset += 12;

    TileRecord record;
    record.tileX = data.tileX;
    record.tileY = data.tileY;
    record.offset = startOffset;

    bool isEmpty = isTileEmpty(data.pixels);

    if (isEmpty) {
        // 空タイル: タイプフラグのみ
        ofs.write(reinterpret_cast<const char*>(&TILE_TYPE_EMPTY), sizeof(uint8_t));
        currentOffset += 1;
        record.type = TILE_TYPE_EMPTY;
        record.size = 0;
    } else {
        // データあり: タイプフラグ + ピクセルデータ
        ofs.write(reinterpret_cast<const char*>(&TILE_TYPE_RAW), sizeof(uint8_t));
        currentOffset += 1;

        ofs.write(reinterpret_cast<const char*>(data.pixels.data()), data.pixels.size());
        currentOffset += data.pixels.size();

        record.type = TILE_TYPE_RAW;
        record.size = data.pixels.size();
    }

    return record;
}

TileData HistoryStorage::readTile(const TileRecord& record) const {
    std::lock_guard<std::mutex> lock(fileMutex);

    TileData tile;
    tile.tileX = record.tileX;
    tile.tileY = record.tileY;

    if (record.type == TILE_TYPE_EMPTY) {
        tile.pixels.resize(tileSize * tileSize * 4, 0);
        return tile;
    }

    std::ifstream ifs(filename, std::ios::binary);
    if (!ifs.is_open()) {
        std::cerr << "Failed to open history file for reading" << std::endl;
        tile.pixels.resize(tileSize * tileSize * 4, 0);
        return tile;
    }

    tile.pixels.resize(tileSize * tileSize * 4);
    
    // ヘッダー(12バイト) + タイプフラグ(1バイト) の後がデータ
    size_t dataOffset = record.offset + 12 + 1;
    ifs.seekg(dataOffset);
    ifs.read(reinterpret_cast<char*>(tile.pixels.data()), record.size);

    if (ifs.gcount() != static_cast<std::streamsize>(record.size)) {
        std::cerr << "Error reading tile data at offset " << dataOffset
                  << " (expected " << record.size << " bytes, got " 
                  << ifs.gcount() << " bytes)" << std::endl;
    }

    return tile;
}

std::vector<TileData> HistoryStorage::readTiles(const std::vector<TileRecord>& records) const {
    std::vector<TileData> result;
    result.reserve(records.size());

    for (const auto& record : records) {
        result.push_back(readTile(record));
    }

    return result;
}

void HistoryStorage::truncate(size_t offset) {
    std::lock_guard<std::mutex> lock(fileMutex);
    ::truncate(filename.c_str(), offset);
    currentOffset = offset;
}
