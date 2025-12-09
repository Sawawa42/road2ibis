# road2ibis

## 導入

- GLFWとGLEWをinstall
  - GLFW: OpenGLの補助ライブラリ。入力やウィンドウを管理する。
  - GLEW: OpenGLの拡張機能取得の補助

```
sudo apt-get -y install libglfw3-dev libglew-dev
```

## memo

- Makefileでは-lGLESv2を利用しているが、libGLESv2.soにes 3.xは含まれているため、#include <GLFW/glfw3.h>で利用可能
