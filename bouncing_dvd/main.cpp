/*

	OpenGL bouncing dvd project
	
	- Used 'stb_image.h' to load png files
	- Mapped them to a square object made of two smaller triangles
	- Learnt to map texture coordinates to object coordinates
	
	- Implemented some functions to move the dvd 
	- Implemented a bounding box system to bounce the dvd logo around

*/

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <shader.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <iostream>
#include <random>

float generateRandomDirection();
void frameBufferSizeCallback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);

const unsigned int SCR_WIDTH = 720;
const unsigned int SCR_HEIGHT = 480;

// formatted for readability, real values are divided by 1000
const float MAX_VELOCITY = 2.0f;
const float MIN_VELOCITY = 1.4f;

int main() {

	// initialize and configure glfw
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// glfw, create window
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "dvd_window", NULL, NULL);
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

	// shader object
	Shader base_shader("glsl/vertex.glsl", "glsl/fragment.glsl");

	// in GPU buffer memory, simplified double triangle implementation using indices and EBO
	float vertices[] = {
		// position[0]		  // colors[1]		  // text coord[2]
		 0.2f,  0.2f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f, // top right
		 0.2f, -0.2f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f, // bottom right
		-0.2f, -0.2f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f, // bottom left
		-0.2f,  0.2f, 0.0f,   1.0f, 0.0f, 0.0f,   0.0f, 1.0f, // top left
	};
	unsigned int indices[] = {
		0, 1, 3,
		1, 2, 3
	};

	// setup GPU memory buffers
	unsigned int VAO, VBO, EBO; // handles
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	/*
	VAO vs VBO vs EBO? What's the difference?
	- https://community.khronos.org/t/opengl-vao-vbo-ebo-explained/75169
	- https://community.khronos.org/t/confirmation-that-i-actually-understand-vaos-vbos-and-ebos/73711

	Vertex Array Objects (VAO)
		- Stores the state related to vertex attribute configurations. 
		- It keeps track of the vertex attribute pointers and the vertex buffer bindings.
		- When you create and bind a VAO, it allows you to store and manage multiple vertex attribute configurations in one object.
		- Once bound, you can quickly switch between different VAOs, which helps in reducing the overhead of state changes when rendering.

	Vertex Buffer Objects (VBO)
		- Stores vertex data (such as positions, normals, colors) directly on the GPU for efficient rendering.
		- When you create a VBO and bind it, you upload vertex data to the GPU. You then set up vertex attribute pointers to specify how the vertex data should be interpreted.
		- Reduces the need to send vertex data to the GPU every frame, improving performance.

	Element Buffer Objects (EBO)
		- Stores indices that specify which vertices to use for drawing primitives (e.g., triangles, lines).
		- When you create an EBO and bind it, you upload index data to the GPU. This allows you to reuse vertices and reduces redundancy i.e. triangles in a polygon.
		- Reduces the amount of vertex data needed and minimizes the number of vertices processed by the GPU

	Summary:
		VAO: Manages the state of vertex attribute pointers and buffers.
		VBO: Stores vertex data on the GPU.
		EBO: Stores index data for efficient drawing of primitives.
	*/

	// vertex array objects (VAO)
	glBindVertexArray(VAO);
	// vertex buffer objects (VBO)
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	// element buffer objects (EBO)
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// access and save buffer attributes
	// position
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// color
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	// texture coordinates
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	// set polygon mode
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	// load image, create texture, and generate mipmaps
	unsigned int texture;
	int width, height, channels;
	stbi_set_flip_vertically_on_load(true);
	unsigned char* data = stbi_load("dvd.png", &width, &height, &channels, 0);
	if (data != NULL) {
		// output debug
		std::cout << "\nLoaded Image with attributes:" << std::endl
			<< "\tWidth     : " << width << std::endl
			<< "\tHeight    : " << width << std::endl
			<< "\tChannels  : " << channels << std::endl;

			// deprecated, flips alpha channel
			//for (int i = 0; i < width * height * channels; i += channels) {
			//	data[i + 3] = 255 - data[i + 3]; // Flip the alpha value
			//}

		// create texture
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture); // operations on GL_TEXTURE_2D now have effect on our texture object through handles
		// set texture wrapping parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		// set texture filtering parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// determine image format
		GLenum format;
		if (channels == 4)
			format = GL_RGBA;
		else if (channels == 3)
			format = GL_RGB;
		else
			format = GL_RED;

		// load texture data into opengl
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		stbi_image_free(data);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	else {
		std::cout << "Failed to load texture" << std::endl;
	}

	// movement variables
	float x_pos = 0.0f;
	float y_pos = 0.0f;
	float x_velocity = generateRandomDirection();
	float y_velocity = generateRandomDirection();
	float object_size_x = 0.2f; // size of bounding box, used for collision
	float object_size_y = 0.2f;

	// render loop
	while (!glfwWindowShouldClose(window)) {
		processInput(window);

		glClearColor(1.0f, 1.0f, 1.0f, 1.0f); // white background
		glClear(GL_COLOR_BUFFER_BIT);

		// update position
		x_pos += x_velocity;
		y_pos += y_velocity;

		// check for edge collision
		if (x_pos + object_size_x > 1.0f || x_pos - object_size_x < -1.0f)
			x_velocity *= -1;
		if (y_pos + object_size_y > 1.0f || y_pos - object_size_y < -1.0f)
			y_velocity *= -1;

		// bind texture
		// ignore warning here, texture is initialized by openGL indirectly
		glBindTexture(GL_TEXTURE_2D, texture);

		// render container
		base_shader.use();

		// pass position to vertex shader
		base_shader.setFloat("x_offset", x_pos);
		base_shader.setFloat("y_offset", y_pos);

		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		// swap buffers and poll IO events
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// cleaning
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);

	glfwTerminate();
	return 0;
}

// generate random movement direction
float generateRandomDirection() {
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<> dis(-MAX_VELOCITY / 1000, MAX_VELOCITY / 1000);

	float num;
	do {
		num = dis(gen);
	} while (num >= -MIN_VELOCITY / 1000 && num <= MIN_VELOCITY / 1000);
	return num;
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