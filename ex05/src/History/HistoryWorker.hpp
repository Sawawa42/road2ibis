#pragma once
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <functional>
#include "HistoryTypes.hpp"

// バックグラウンドスレッドでのタイル書き込みを管理
class HistoryWorker {
public:
    using WriteCallback = std::function<TileRecord(const TileData&)>;

    HistoryWorker();
    ~HistoryWorker();

    // ワーカースレッドを開始
    void start(WriteCallback callback);

    // ワーカースレッドを停止
    void stop();

    // 書き込みタスクをキューに追加
    void enqueue(TileData&& data);

    // キューが空になるまで待機
    void waitUntilEmpty();

    // 処理完了したレコードを取得（コールバック経由で通知）
    using RecordCallback = std::function<void(int stepID, const TileRecord&)>;
    void setRecordCallback(RecordCallback callback) { recordCallback = callback; }

private:
    std::thread workerThread;
    std::atomic<bool> isRunning{false};

    std::queue<TileData> workQueue;
    std::mutex queueMutex;
    std::condition_variable queueCond;
    std::condition_variable emptyCondition;

    WriteCallback writeCallback;
    RecordCallback recordCallback;

    void workerLoop();
};
