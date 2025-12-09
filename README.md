# road2ibis

## 導入

- GLFWとGLEWをinstall
  - GLFW: OpenGLの補助ライブラリ。入力やウィンドウを管理する。グラフィックライブラリフレームワーク
    - GLUT(OpenGL Utility Toolkit)という1990年代の古いものもある
  - GLEW: OpenGLの拡張機能取得の補助　GL Extension Wrangler Library
    - 42Tokyo校舎では利用不可

```
sudo apt-get -y install libglfw3-dev libglew-dev
```

## memo

- Makefileでは-lGLESv2を利用しているが、libGLESv2.soにes 3.xは含まれているため、#include <GLFW/glfw3.h>で利用可能
