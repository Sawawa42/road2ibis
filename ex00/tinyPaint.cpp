#include "tinyPaint.hpp"

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

TinyPaint::TinyPaint(GLuint width, GLuint height, const char* title): \
	_windowWidth(width), _windowHeight(height), _title(title) {
	// GLFWの初期化
	if (!glfwInit()) {
		throw std::runtime_error("GLFW initialization failed");
	}

	// OpenGLのバージョンを指定 (2.1)
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

	_window = glfwCreateWindow(_windowWidth, _windowHeight, _title, nullptr, nullptr);
	if (!_window) {
		glfwTerminate();
		throw std::runtime_error("Window creation failed");
	}

	glfwMakeContextCurrent(_window);
	glfwSetKeyCallback(_window, key_callback);

	// glewExperimental = GL_TRUE;
	// if (glewInit() != GLEW_OK) {
	// 	glfwDestroyWindow(_window);
	// 	glfwTerminate();
	// 	throw std::runtime_error("GLEW initialization failed");
	// }
}

TinyPaint::~TinyPaint() {
	glfwDestroyWindow(_window);
	glfwTerminate();
}

void TinyPaint::setup() {
	glViewport(0, 0, _windowWidth, _windowHeight);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-1.0, 1.0, -1.0, 1.0, -1.0, 1.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

// メインループ
void TinyPaint::loop() {
	while (!glfwWindowShouldClose(_window)) {
		glfwPollEvents();

		// 背景色の設定 (黒) とクリア
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		// 三角形を描画
		glBegin(GL_TRIANGLES);
			// 赤色を設定
			glColor3f(1.0f, 0.0f, 0.0f); // R, G, B

			// 頂点座標 (画面中央に配置)
			// OpenGLの座標系は通常、左下隅が (-1, -1)、右上隅が (1, 1)
			glVertex2f(0.0f,  0.5f);  // 上の頂点
			glVertex2f(-0.5f, -0.5f); // 左下の頂点
			glVertex2f(0.5f, -0.5f);  // 右下の頂点
		glEnd();

		glfwSwapBuffers(_window);
	}
}

// キー入力時のコールバック関数
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	(void)scancode;
	(void)mods;
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, GL_TRUE);
	}
}
