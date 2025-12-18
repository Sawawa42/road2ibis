#include "HistoryWorker.hpp"

HistoryWorker::HistoryWorker() = default;

HistoryWorker::~HistoryWorker() {
    stop();
}

void HistoryWorker::start(WriteCallback callback) {
    writeCallback = std::move(callback);
    isRunning = true;
    workerThread = std::thread(&HistoryWorker::workerLoop, this);
}

void HistoryWorker::stop() {
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        isRunning = false;
    }
    queueCond.notify_all();

    if (workerThread.joinable()) {
        workerThread.join();
    }
}

void HistoryWorker::enqueue(TileData&& data) {
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        workQueue.push(std::move(data));
    }
    queueCond.notify_one();
}

void HistoryWorker::waitUntilEmpty() {
    std::unique_lock<std::mutex> lock(queueMutex);
    emptyCondition.wait(lock, [this]() { return workQueue.empty(); });
}

void HistoryWorker::workerLoop() {
    while (true) {
        TileData data;
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            queueCond.wait(lock, [this]() { 
                return !workQueue.empty() || !isRunning; 
            });

            if (!isRunning && workQueue.empty()) {
                break;
            }

            data = std::move(workQueue.front());
            workQueue.pop();

            // キューが空になったら通知
            if (workQueue.empty()) {
                emptyCondition.notify_all();
            }
        }

        // ファイル書き込み実行
        if (writeCallback) {
            TileRecord record = writeCallback(data);

            // レコード情報をコールバックで通知
            if (recordCallback) {
                recordCallback(data.stepID, record);
            }
        }
    }
}
