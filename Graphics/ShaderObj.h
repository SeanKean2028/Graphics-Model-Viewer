#pragma once

#include <GL/glew.h>
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
using namespace std;
//Attribute pointer data to set up buffers properly
struct AttributePointerData {
	//How many of x data type to iterate over
	int size;
	//Type of our data
	GLenum type;
	//Is normalized
	GLboolean normalized;
	//Stride steps between each
	GLsizei stride;
	//Name of attribute set
	GLchar* name;
	//Offset to next
	const void* offset;
	AttributePointerData(int _size, GLenum _type, GLboolean _normalized, GLsizei _stride, GLchar* _name, const void* _offset) {
		size = _size;
		type = _type;
		normalized = _normalized;
		stride = _stride;
		name = _name;
		offset = _offset;
	}
};
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
	vector<AttributePointerData> attributePointerDatas;
	GLuint program;
	//Sets all attribute pointers so buffer knows how to use our shaders in memory
	void SetAttributePointers() {
		for (AttributePointerData p : attributePointerDatas) {
			GLint _id = glGetAttribLocation(program, p.name);
			glEnableVertexAttribArray(_id);
			glVertexAttribPointer(_id, p.size, p.type, p.normalized, p.stride, p.offset);
		}
	}
	//Adds attribute pointer so buffer knows how to use our shader in memory int size, GLenum type, GLboolean normalized, GLsizei stride, GLchar* name, const void* offset 
	void AddAttributePointer(int size, GLenum type, GLboolean normalized, GLsizei stride, GLchar* name, const void* offset ) {
		AttributePointerData atb = AttributePointerData(size, type, normalized, stride, name, offset);
		attributePointerDatas.push_back(atb);
	}

};
//Doesn't need attributes, 
class FragmentShader : public Shader{
public:
	using Shader::Shader;
};