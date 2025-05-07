#ifndef TINYPAINT_HPP
#define TINYPAINT_HPP

#include <iostream>
#include <string>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

class TinyPaint {
	public:
		TinyPaint(GLuint width, GLuint height, const char* title);
		~TinyPaint();
		void setup();
		void loop();
	
	private:
		GLFWwindow* _window;
		const GLuint _windowWidth, _windowHeight;
		const char* _title;
};

#endif
