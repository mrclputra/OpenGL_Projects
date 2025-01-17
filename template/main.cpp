/*

	OpenGL base template made by Marcelino Ares Putra, 7th August 2024
	
	Place vertex and fragment glsl files in the glsl folder when you create them
	Should new libraries or headers be needed, don't forget to update 'include' directories in project settings

	'shader.h' is an automated implementation to generate shader API handles from vertex and fragment files
	it returns shader program handle

	use my custom .gitignore file

*/

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>

void frameBufferSizeCallback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);

const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

int main() {

	// initialize and configure glfw
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// glfw, create window
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "temp_window", NULL, NULL);
	if (window == NULL) {
		std::cout << "Failed to create a GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, frameBufferSizeCallback);

	// load GLAD opengl function pointers
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	// render loop
	while (!glfwWindowShouldClose(window)) {
		processInput(window);

		// swap buffers and poll IO events
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}

// process window inputs
void processInput(GLFWwindow* window) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	// additional inputs here
}

// handle window resizing
void frameBufferSizeCallback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
}