#pragma once
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <string>
#include <fstream>
#include <iostream>

struct TileData {
    int tileX, tileY;
    int stepID;
    std::vector<uint8_t> pixels;
};

class UndoSystem {
    public:
        UndoSystem(const std::string& filename, int tileSize);
        ~UndoSystem();

        void pushTile(int tileX, int tileY, int stepID, const uint8_t* data);

        int getCurrentStepID() const { return currentStepID; }
        void incrementStepID() { currentStepID++; }

    private:
        std::string historyFile;
        int tileSize;
        std::atomic<int> currentStepID;

        std::thread workerThread;
        std::atomic<bool> isRunning;

        std::queue<TileData> workQueue;
        std::mutex queueMutex;
        std::condition_variable queueCond;

        void workerLoop();

        void appendToFile(const TileData& data, std::ofstream& ofs);
};
