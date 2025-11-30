#pragma once

#include <GL/glew.h>
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
using namespace std;

class Shader {
public:
	GLuint id;
	string filePath;
	GLenum type;
	// Load shader code from file. Just inser type: either GL_VERTEX_SHADER, or GL_FRAGMENT_SHADER
	Shader(string _filePath, GLenum _type) {
		filePath = _filePath;
		type = _type;
	}
	GLuint compileShader() {
		//Open File
		std::ifstream file(filePath);
		if (!file.is_open()) {
			std::cerr << "ERROR: Cannot open shader file: " << filePath << std::endl;
			return 0;
		}
		//Read file
		std::stringstream ss;
		ss << file.rdbuf();
		std::string codeStr = ss.str();
		const char* code = codeStr.c_str();
		// Create and compile shader
		GLuint shader = glCreateShader(type);
		glShaderSource(shader, 1, &code, nullptr);
		glCompileShader(shader);

		// Check compilation
		GLint success;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
		if (!success) {
			char infoLog[512];
			glGetShaderInfoLog(shader, 512, nullptr, infoLog);
			std::cerr << "ERROR: Shader compilation failed ("
				<< filePath << "):\n" << infoLog << std::endl;
		}

		return shader;
	}
	//Delte our shader
	void Delete() {
		glDeleteShader(id);
	}
};
class VertexShader : public Shader{
public: 
	using Shader::Shader;
};
//Doesn't need attributes, 
class FragmentShader : public Shader{
public:
	using Shader::Shader;
};