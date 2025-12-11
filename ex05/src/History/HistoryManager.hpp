#pragma once
#include <memory>
#include <map>
#include <vector>
#include <atomic>
#include <mutex>
#include "HistoryTypes.hpp"
#include "HistoryStorage.hpp"
#include "HistoryWorker.hpp"

// 履歴管理: Undo/Redoロジックを担当
class HistoryManager {
public:
    HistoryManager(const std::string& filename, int tileSize);
    ~HistoryManager();

    // タイルデータの保存（描画前: Undo用）
    void pushBeforeTile(int tileX, int tileY, int stepID, const uint8_t* data);

    // タイルデータの保存（描画後: Redo用）
    void pushAfterTile(int tileX, int tileY, int stepID, const uint8_t* data);

    // stepID管理
    int getCurrentStepID() const { return currentStepID.load(); }
    void incrementStepID();

    // Undo/Redo操作
    std::vector<TileData> undo();
    std::vector<TileData> redo();

    bool canUndo() const { return currentStepID.load() > 0; }
    bool canRedo() const { return currentStepID.load() < maxStepID.load(); }

    // ワーカースレッドの同期
    void waitForPendingWrites();

private:
    int tileSize;
    std::atomic<int> currentStepID{0};
    std::atomic<int> maxStepID{0};

    std::unique_ptr<HistoryStorage> storage;
    std::unique_ptr<HistoryWorker> worker;

    // インデックスマップ（stepID -> タイルレコード一覧）
    std::map<int, std::vector<TileRecord>> beforeIndex;  // Undo用
    std::map<int, std::vector<TileRecord>> afterIndex;   // Redo用
    std::mutex indexMutex;

    // インデックスにレコードを追加（ワーカーからのコールバック用）
    void onBeforeTileWritten(int stepID, const TileRecord& record);

    // 不要な履歴を削除
    void clearHistoryAfter(int stepID);

    // 最大オフセットを計算
    size_t calculateMaxOffset(int upToStepID) const;
};
