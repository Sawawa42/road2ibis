#include "HistoryManager.hpp"
#include <iostream>

HistoryManager::HistoryManager(const std::string& filename, int tileSize)
    : tileSize(tileSize) {
    storage = std::make_unique<HistoryStorage>(filename, tileSize);
    worker = std::make_unique<HistoryWorker>();

    // ワーカースレッドを開始し、書き込み完了時のコールバックを設定
    worker->start([this](const TileData& data) {
        return storage->writeTile(data);
    });

    worker->setRecordCallback([this](int stepID, const TileRecord& record) {
        onBeforeTileWritten(stepID, record);
    });
}

HistoryManager::~HistoryManager() {
    worker->stop();
}

void HistoryManager::pushBeforeTile(int tileX, int tileY, int stepID, const uint8_t* data) {
    TileData tileData;
    tileData.tileX = tileX;
    tileData.tileY = tileY;
    tileData.stepID = stepID;
    tileData.pixels.assign(data, data + tileSize * tileSize * 4);

    // バックグラウンドで書き込み
    worker->enqueue(std::move(tileData));
}

void HistoryManager::pushAfterTile(int tileX, int tileY, int stepID, const uint8_t* data) {
    TileData tileData;
    tileData.tileX = tileX;
    tileData.tileY = tileY;
    tileData.stepID = stepID;
    tileData.pixels.assign(data, data + tileSize * tileSize * 4);

    // 同期的に書き込み(ストローク終了時なので即時保存)
    TileRecord record = storage->writeTile(tileData);

    {
        std::lock_guard<std::mutex> lock(indexMutex);
        afterIndex[stepID].push_back(record);
    }
}

void HistoryManager::onBeforeTileWritten(int stepID, const TileRecord& record) {
    std::lock_guard<std::mutex> lock(indexMutex);
    beforeIndex[stepID].push_back(record);
}

void HistoryManager::incrementStepID() {
    currentStepID++;
    int newStepID = currentStepID.load();

    // 新しいストローク開始時、現在のstepID以降の履歴を削除
    clearHistoryAfter(newStepID);

    maxStepID.store(newStepID);
}

void HistoryManager::clearHistoryAfter(int stepID) {
    std::lock_guard<std::mutex> lock(indexMutex);

    // stepID以上のエントリを削除
    auto beforeIt = beforeIndex.begin();
    while (beforeIt != beforeIndex.end()) {
        if (beforeIt->first >= stepID) {
            beforeIt = beforeIndex.erase(beforeIt);
        } else {
            ++beforeIt;
        }
    }

    auto afterIt = afterIndex.begin();
    while (afterIt != afterIndex.end()) {
        if (afterIt->first >= stepID) {
            afterIt = afterIndex.erase(afterIt);
        } else {
            ++afterIt;
        }
    }

    // ファイルを切り詰め
    size_t truncateOffset = calculateMaxOffset(stepID);
    storage->truncate(truncateOffset);
}

size_t HistoryManager::calculateMaxOffset(int upToStepID) const {
    size_t maxOffset = 0;

    for (const auto& entry : beforeIndex) {
        if (entry.first < upToStepID) {
            for (const auto& record : entry.second) {
                size_t endOffset = record.offset + 12 + 1;  // ヘッダー + タイプフラグ
                if (record.type == TILE_TYPE_RAW) {
                    endOffset += record.size;
                }
                maxOffset = std::max(maxOffset, endOffset);
            }
        }
    }

    for (const auto& entry : afterIndex) {
        if (entry.first < upToStepID) {
            for (const auto& record : entry.second) {
                size_t endOffset = record.offset + 12 + 1;
                if (record.type == TILE_TYPE_RAW) {
                    endOffset += record.size;
                }
                maxOffset = std::max(maxOffset, endOffset);
            }
        }
    }

    return maxOffset;
}

std::vector<TileData> HistoryManager::undo() {
    waitForPendingWrites();

    std::vector<TileData> result;
    int targetStepID = currentStepID.load();

    if (targetStepID <= 0) {
        return result;
    }

    std::vector<TileRecord> records;
    {
        std::lock_guard<std::mutex> lock(indexMutex);
        auto it = beforeIndex.find(targetStepID);
        if (it == beforeIndex.end()) {
            std::cerr << "No records found for stepID " << targetStepID << std::endl;
            currentStepID--;
            return result;
        }
        records = it->second;
        std::cout << "Undoing stepID " << targetStepID << " with " << records.size() << " tiles." << std::endl;
        currentStepID--;
    }

    // タイルデータを読み込み
    result = storage->readTiles(records);
    for (auto& tile : result) {
        tile.stepID = targetStepID;
    }

    return result;
}

std::vector<TileData> HistoryManager::redo() {
    waitForPendingWrites();

    std::vector<TileData> result;
    int targetStepID = currentStepID.load() + 1;

    if (targetStepID > maxStepID.load()) {
        return result;
    }

    std::vector<TileRecord> records;
    {
        std::lock_guard<std::mutex> lock(indexMutex);
        auto it = afterIndex.find(targetStepID);
        if (it == afterIndex.end()) {
            std::cerr << "No after-records found for stepID " << targetStepID << std::endl;
            return result;
        }
        records = it->second;
        std::cout << "Redoing stepID " << targetStepID << " with " << records.size() << " tiles." << std::endl;
        currentStepID++;
    }

    // タイルデータを読み込み
    result = storage->readTiles(records);
    for (auto& tile : result) {
        tile.stepID = targetStepID;
    }

    return result;
}

void HistoryManager::waitForPendingWrites() {
    worker->waitUntilEmpty();
}
