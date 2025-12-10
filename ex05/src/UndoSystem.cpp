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
    std::ofstream ofs(historyFile, std::ios::binary | std::ios::app);

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

        appendToFile(tileData, ofs);
    }
}

void UndoSystem::appendToFile(const TileData& data, std::ofstream& ofs) {
    ofs.write(reinterpret_cast<const char*>(&data.stepID), sizeof(int));
    ofs.write(reinterpret_cast<const char*>(&data.tileX), sizeof(int));
    ofs.write(reinterpret_cast<const char*>(&data.tileY), sizeof(int));
    ofs.write(reinterpret_cast<const char*>(data.pixels.data()), data.pixels.size());
}
