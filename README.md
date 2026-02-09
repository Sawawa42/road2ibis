# road2ibis

Road to ibis tinyPaintは、C++とOpenGLを用いて実装したシンプルなペイントアプリケーションで、ブラシ色の変更機能、画像の保存機能、Undo/Redo機能を備えています。

<img src="./images/tinypaint.png" width="60%">

## 実行方法

前提として、libglew-devやlibglfw3-devのインストールが必要。42Tokyo校舎の環境ではこれらはインストール済みとなっている。

```
// lodepng用
git submodule update --init --recursive

// コンパイル
make

// 実行
./tinyPaint
```

## 操作方法

```
マウス左ボタン: 線の描画を行う。

1,R,G,B,0キー: それぞれ、黒、赤、緑、青、透明(消しゴム)に対応。デフォルトは黒。

Sキー: 画像をoutput.pngとして保存する。

Ctrl+Z/Ctrl+Y: Undo/Redoを行う。
```
