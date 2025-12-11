#include "InputManager.hpp"

InputManager::InputManager(GLFWwindow* window)
    : window(window) {
}

void InputManager::update() {
    // マウス状態更新
    glfwGetCursorPos(window, &mouseState.x, &mouseState.y);
    bool currentLeftPressed = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;

    mouseState.leftJustPressed = currentLeftPressed && !prevLeftPressed;
    mouseState.leftJustReleased = !currentLeftPressed && prevLeftPressed;
    mouseState.leftPressed = currentLeftPressed;

    prevLeftPressed = currentLeftPressed;
}

bool InputManager::isKeyPressed(int key) const {
    return glfwGetKey(window, key) == GLFW_PRESS;
}

bool InputManager::isKeyTriggered(int key) {
    bool currentPressed = isKeyPressed(key);
    bool wasPressed = prevKeyState[key];
    prevKeyState[key] = currentPressed;
    return currentPressed && !wasPressed;
}

bool InputManager::isCtrlPressed() const {
    return isKeyPressed(GLFW_KEY_LEFT_CONTROL) || isKeyPressed(GLFW_KEY_RIGHT_CONTROL);
}

InputAction InputManager::getTriggeredAction() const {
    // Note: const版なのでprevKeyStateを更新しない簡易実装
    // トリガー検出は別途管理が必要

    if (isCtrlPressed()) {
        if (isKeyPressed(GLFW_KEY_Z)) {
            return InputAction::Undo;
        }
        if (isKeyPressed(GLFW_KEY_Y)) {
            return InputAction::Redo;
        }
    }

    if (isKeyPressed(GLFW_KEY_S)) {
        return InputAction::Save;
    }
    if (isKeyPressed(GLFW_KEY_0)) {
        return InputAction::SetEraser;
    }
    if (isKeyPressed(GLFW_KEY_1)) {
        return InputAction::SetBrushBlack;
    }
    if (isKeyPressed(GLFW_KEY_R)) {
        return InputAction::SetBrushRed;
    }
    if (isKeyPressed(GLFW_KEY_G)) {
        return InputAction::SetBrushGreen;
    }
    if (isKeyPressed(GLFW_KEY_B)) {
        return InputAction::SetBrushBlue;
    }

    return InputAction::None;
}
