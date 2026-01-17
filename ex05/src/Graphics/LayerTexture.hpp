#pragma once
#include <GL/glew.h>
#include <vector>
#include <cstdint>

// レイヤーテクスチャ: 描画データを格納する2次元メモリ領域
class LayerTexture {
public:
    LayerTexture(int width, int height);
    ~LayerTexture();

    // コピー禁止
    LayerTexture(const LayerTexture&) = delete;
    LayerTexture& operator=(const LayerTexture&) = delete;

    // ムーブ許可
    LayerTexture(LayerTexture&& other) noexcept;
    LayerTexture& operator=(LayerTexture&& other) noexcept;

    GLuint getId() const { return textureId; }
    int getWidth() const { return width; }
    int getHeight() const { return height; }

    // タイルの部分更新
    void updateTile(int x, int y, int tileWidth, int tileHeight, const uint8_t* data);

    // 全体クリア
    void clear(float r, float g, float b, float a);

    // 全ピクセル読み取り
    std::vector<uint8_t> readAllPixels() const;

private:
    GLuint textureId = 0;
    int width = 0;
    int height = 0;
};
