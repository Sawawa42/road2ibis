# include "tinyPaint.hpp"

int main() {
	try {
		TinyPaint tinyPaint(800, 600, "tiniPaint");

		tinyPaint.setup();
		tinyPaint.loop();
	} catch (const std::exception& e) {
		std::cerr << "Error: " << e.what() << std::endl;
		return EXIT_FAILURE;
	} catch (...) {
		std::cerr << "Unknown error occurred" << std::endl;
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
