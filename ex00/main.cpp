#include "tinyPaint.hpp"

OpenGLFunctions gl;

int main(void) {
    try {
        TinyPaint app(400, 400, "TinyPaint");
        if (app.init() != 0) {
            std::cerr << "Failed to initialize TinyPaint" << std::endl;
            return -1;
        }
        app.run();
        app.cleanup();
    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
