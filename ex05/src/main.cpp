#include "Core/App.hpp"

int main() {
    try {
        App app(800, 600, "tinyPaint", 4096.0f);
        app.run();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
