#include "UndoSystem.hpp"

UndoSystem::UndoSystem(const std::string& filename, int tileSize)
    : historyFile(filename), tileSize(tileSize), currentStepID(0), isRunning(true) {

    std::ofstream ofs(historyFile, std::ios::binary | std::ios::trunc);
    ofs.close();

    workerThread = std::thread(&UndoSystem::workerLoop, this);
}

UndoSystem::~UndoSystem() {
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        isRunning = false;
    }
    queueCond.notify_all();
    if (workerThread.joinable()) {
        workerThread.join();
    }
}

void UndoSystem::pushTile(int tileX, int tileY, int stepID, const uint8_t* data) {
    TileData tileData;
    tileData.tileX = tileX;
    tileData.tileY = tileY;
    tileData.stepID = stepID;
    tileData.pixels.assign(data, data + tileSize * tileSize * 4);

    {
        std::lock_guard<std::mutex> lock(queueMutex);
        workQueue.push(std::move(tileData));
    }
    queueCond.notify_one();
}

void UndoSystem::workerLoop() {
    while (true) {
        TileData tileData;
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            queueCond.wait(lock, [this]() { return !workQueue.empty() || !isRunning; });

            if (!isRunning && workQueue.empty()) {
                break;
            }

            tileData = std::move(workQueue.front());
            workQueue.pop();
        }

        {
            std::ofstream ofs(historyFile, std::ios::binary | std::ios::app);
            appendToFile(tileData, ofs);
        }
    }
}

const uint8_t TYPE_EMPTY = 0;
const uint8_t TYPE_RAW = 1;

void UndoSystem::appendToFile(const TileData& data, std::ofstream& ofs) {
    size_t startOffset = currentFileOffset;

    ofs.write(reinterpret_cast<const char*>(&data.stepID), sizeof(int));
    ofs.write(reinterpret_cast<const char*>(&data.tileX), sizeof(int));
    ofs.write(reinterpret_cast<const char*>(&data.tileY), sizeof(int));
    currentFileOffset += 12;

    bool isEmpty = true;
    size_t dataSize = data.pixels.size() / 4;
    for (size_t i = 0; i < dataSize; ++i) {
        if (data.pixels[i * 4 + 3] != 0) { // Alphaチャネルが0でないピクセルがある場合
            isEmpty = false;
            break;;
        }
    }

    TileRecord record;
    record.tileX = data.tileX;
    record.tileY = data.tileY;
    record.offset = startOffset;

    if (isEmpty) {
        ofs.write(reinterpret_cast<const char*>(&TYPE_EMPTY), sizeof(uint8_t));
        currentFileOffset += 1;
        record.type = TYPE_EMPTY;
        record.size = 0;
    } else {
        ofs.write(reinterpret_cast<const char*>(&TYPE_RAW), sizeof(uint8_t));
        currentFileOffset += 1;

        ofs.write(reinterpret_cast<const char*>(data.pixels.data()), data.pixels.size());
        currentFileOffset += data.pixels.size();

        record.type = TYPE_RAW;
        record.size = data.pixels.size();
    }

    {
        std::lock_guard<std::mutex> lock(queueMutex);
        indexMap[data.stepID].push_back(record);
    }
}

std::vector<TileData> UndoSystem::undo() {
    waitWorker();

    std::vector<TileData> result;
    int targetStepID = currentStepID;

    if (targetStepID <= 0) {
        return result;
    }

    std::vector<TileRecord> records;
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        if (indexMap.find(targetStepID) == indexMap.end()) {
            std::cerr << "No records found for stepID " << targetStepID << std::endl;
            currentStepID--;
            return result;
        }
        records = indexMap[targetStepID];

        std::cout << "Undoing stepID " << targetStepID << " with " << records.size() << " tiles." << std::endl;

        indexMap.erase(targetStepID); // redo対応ならここで削除しない
        currentStepID--;
    }

    std::ifstream ifs(historyFile, std::ios::binary);
    for (const auto& record : records) {
        TileData tile;
        tile.tileX = record.tileX;
        tile.tileY = record.tileY;
        tile.stepID = targetStepID;

        if (record.type == TYPE_EMPTY) {
            tile.pixels.resize(tileSize * tileSize * 4, 0);
        } else if (record.type == TYPE_RAW) {
            tile.pixels.resize(tileSize * tileSize * 4);
            ifs.seekg(record.offset + 12 + 1);
            ifs.read(reinterpret_cast<char*>(tile.pixels.data()), record.size);
            if (ifs.gcount() != static_cast<std::streamsize>(record.size)) {
                std::cerr << "Error reading tile data at offset " << record.offset << std::endl;
            }
        }
        result.push_back(std::move(tile));
    }

    return result;
}

void UndoSystem::waitWorker() {
    std::unique_lock<std::mutex> lock(queueMutex);
    queueCond.wait(lock, [this]() { return workQueue.empty(); });
}
