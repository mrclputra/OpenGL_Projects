#pragma once
// shader loader class defined here
// used to read files in 'glsl' folder

#include <glad/glad.h>

#include <string>
#include <fstream>
#include <sstream>

#include <iostream>

class Shader {
public:
	unsigned int program; // API program handle

	// constructor to build shader program on the fly
	Shader(const char* vertex_path, const char* fragment_path) {

		std::string vertex_code;
		std::string fragment_code;
		std::ifstream v_shader_file;
		std::ifstream f_shader_file;

		// ensure ifstream objects can throw exceptions
		v_shader_file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		f_shader_file.exceptions(std::ifstream::failbit | std::ifstream::badbit);

		try {
			v_shader_file.open(vertex_path);
			f_shader_file.open(fragment_path);
			std::stringstream v_shader_stream, f_shader_stream;

			v_shader_stream << v_shader_file.rdbuf();
			f_shader_stream << f_shader_file.rdbuf();

			v_shader_file.close();
			f_shader_file.close();

			vertex_code = v_shader_stream.str();
			fragment_code = f_shader_stream.str();

			// debug print source code contents, ensure both files are loaded sucessfully
			std::cout << "\n** vertex shader source code **" << std::endl;
			std::cout << vertex_code << std::endl;
			std::cout << "\n** fragment shader source code **" << std::endl;
			std::cout << fragment_code << std::endl;
		}
		catch (std::ifstream::failure& e) {
			std::cout << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ: " << e.what() << std::endl;
		}

		const char* v_shader_code = vertex_code.c_str();
		const char* f_shader_code = fragment_code.c_str();

		// compile shader
		unsigned int vertex, fragment; // shader API handles

		vertex = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertex, 1, &v_shader_code, NULL);
		glCompileShader(vertex);
		checkCompileErrors(vertex, "VERTEX");

		fragment = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragment, 1, &f_shader_code, NULL);
		glCompileShader(fragment);
		checkCompileErrors(fragment, "FRAGMENT");

		program = glCreateProgram();
		glAttachShader(program, vertex);
		glAttachShader(program, fragment);
		glLinkProgram(program);
		checkCompileErrors(program, "PROGRAM");

		glDeleteShader(vertex);
		glDeleteShader(fragment);
	}

	// activate shader
	void use() {
		glUseProgram(program);
	}

	// utility uniform functions, to change values within shader program
	void setBool(const std::string& name, bool value) const
	{
		glUniform1i(glGetUniformLocation(program, name.c_str()), (int)value);
	}
	void setInt(const std::string& name, int value) const
	{
		glUniform1i(glGetUniformLocation(program, name.c_str()), value);
	}
	void setFloat(const std::string& name, float value) const
	{
		glUniform1f(glGetUniformLocation(program, name.c_str()), value);
	}

private:
	// utility function to check for compile/linking errors
	void checkCompileErrors(unsigned int shader, std::string type) {
		// flags
		int success;
		char infoLog[1024];

		if (type != "PROGRAM") {
			glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
			if (!success) {
				glGetShaderInfoLog(shader, 1024, NULL, infoLog);
				std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
			}
		}
		else {
			glGetProgramiv(shader, GL_LINK_STATUS, &success);
			if (!success) {
				glGetProgramInfoLog(shader, 1024, NULL, infoLog);
				std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
			}
		}
	}

};