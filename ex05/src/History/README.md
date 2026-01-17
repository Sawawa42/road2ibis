# History

Undo/Redo機能を実現する履歴管理システム

## HistoryManagerクラス

- Undo/Redoのメインロジックを担当
- stepID(操作ステップ番号)の管理
- タイルデータの保存(描画前: Undo用、描画後: Redo用)
- undo()/redo()で復元データを返却
- ワーカースレッドとストレージの統合管理

## HistoryStorageクラス

- ファイルI/Oによる永続化層
- タイルデータの書き込み・読み込み
- 空タイルの検出と最適化(TYPE_EMPTYとTYPE_RAW)
- ファイルの切り詰め(truncate)とクリア

## HistoryWorkerクラス

- バックグラウンドスレッドでの非同期書き込み
- 書き込みタスクのキュー管理
- 完了通知のコールバック機構
- waitUntilEmpty()で同期待機

## HistoryTypes.hpp

- 履歴システムで使用する共通データ構造の定義
- TileData: タイル座標、stepID、ピクセルデータ
- TileRecord: ファイル内の記録情報(オフセット、サイズ、タイプ)
- タイルタイプ定数(TILE_TYPE_EMPTY, TILE_TYPE_RAW)
