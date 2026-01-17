#pragma once
#include <GLFW/glfw3.h>
#include <functional>
#include <unordered_map>

// 入力アクションの種類
enum class InputAction {
    None,
    Undo,
    Redo,
    Save,
    SetBrushBlack,
    SetBrushRed,
    SetBrushGreen,
    SetBrushBlue,
    SetEraser
};

// マウス状態
struct MouseState {
    double x = 0.0;
    double y = 0.0;
    bool leftPressed = false;
    bool leftJustPressed = false;   // このフレームで押された
    bool leftJustReleased = false;  // このフレームで離された
};

class InputManager {
public:
    InputManager(GLFWwindow* window);

    void update();

    // キーボード入力
    InputAction getTriggeredAction() const;

    // マウス入力
    const MouseState& getMouseState() const { return mouseState; }

private:
    GLFWwindow* window;
    MouseState mouseState;
    bool prevLeftPressed = false;

    // キー状態管理(トリガー検出用)
    std::unordered_map<int, bool> prevKeyState;

    bool isKeyTriggered(int key);
    bool isKeyPressed(int key) const;
    bool isCtrlPressed() const;
};
