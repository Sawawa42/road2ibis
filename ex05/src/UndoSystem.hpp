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
#include <map>

struct TileData {
    int tileX, tileY;
    int stepID;
    std::vector<uint8_t> pixels;
};

struct TileRecord {
    int tileX, tileY;
    uint8_t type; // 0: TYPE_EMPTY, 1: TYPE_RAW
    size_t offset;
    size_t size;
};

class UndoSystem {
    public:
        UndoSystem(const std::string& filename, int tileSize);
        ~UndoSystem();

        void pushTile(int tileX, int tileY, int stepID, const uint8_t* data);

        int getCurrentStepID() const { return currentStepID; }
        void incrementStepID() { currentStepID++; }

        std::vector<TileData> undo();
        bool canUndo() const { return currentStepID > 0; }

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

        std::map<int, std::vector<TileRecord>> indexMap;
        size_t currentFileOffset = 0;

        TileData loadTileFromRecord(const TileRecord& record);
};
