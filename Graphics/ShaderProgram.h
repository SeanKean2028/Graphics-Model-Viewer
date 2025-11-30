#pragma once
#ifndef SHADER_H
#define SHADER_H
#include <gtc/type_ptr.hpp>
#include <GL/glew.h>
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <glm.hpp>
#include <vector>
#include "ShaderObj.h"
// DO NOT include "Mesh.h" here - creates circular dependency!

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
	std::string name;
	//Offset to next
	const void* offset;

	AttributePointerData(int _size, GLenum _type, GLboolean _normalized, GLsizei _stride, const std::string& _name, const void* _offset)
		: size(_size), type(_type), normalized(_normalized), stride(_stride), name(_name), offset(_offset) {
	}
};

class ShaderProgram
{
public:
	unsigned int ID;
	vector<AttributePointerData> attributePointerDatas;

	//Adds attribute pointer so buffer knows how to use our shader in memory
	void AddAttributePointer(int size, GLenum type, GLboolean normalized, GLsizei stride, const std::string& name, const void* offset) {
		attributePointerDatas.push_back(AttributePointerData(size, type, normalized, stride, name, offset));
	}

	//Sets all attribute pointers so buffer knows how to use our shaders in memory
	void SetAttributePointers() {
		for (const AttributePointerData& p : attributePointerDatas) {
			GLint _id = glGetAttribLocation(ID, p.name.c_str());
			if (_id < 0) {
				std::cerr << "Warning: attribute '" << p.name << "' not found in program " << ID << std::endl;
				continue;
			}
			glEnableVertexAttribArray(_id);
			glVertexAttribPointer(_id, p.size, p.type, p.normalized, p.stride, p.offset);
		}
	}

	ShaderProgram(VertexShader& vertexShader, FragmentShader& fragmentShader)
	{
		// Compiles both shaders and sets their GLuint ids
		vertexShader.id = vertexShader.compileShader();
		fragmentShader.id = fragmentShader.compileShader();

		// Create shader program
		ID = glCreateProgram();
		glAttachShader(ID, vertexShader.id);
		glAttachShader(ID, fragmentShader.id);
		glBindFragDataLocation(ID, 0, "outColor");
		glLinkProgram(ID);

		//Check if linking worked
		GLint success;
		char infoLog[512];
		glGetProgramiv(ID, GL_LINK_STATUS, &success);
		if (!success) {
			glGetProgramInfoLog(ID, 512, NULL, infoLog);
			std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
		}
	}

	ShaderProgram() {
	}

	//Set ShaderProgram as current glfw shader
	void use() {
		glUseProgram(ID);
	}

	//Disposes program
	void Delete() {
		glDeleteProgram(ID);
	}

	//Sets bool uniform with name and value
	void setBool(const std::string& name, bool value) const {
		glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
	}

	//Sets int uniform with name and value
	void setInt(const std::string& name, int value) const {
		glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
	}

	//Sets float uniform with name and value
	void setFloat(const std::string& name, float value) const {
		glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
	}

	//Sets Matrix 4 uniform with name and value
	void setMat4(const std::string& name, const glm::mat4& mat) const {
		glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, glm::value_ptr(mat));
	}

	//Sets Vec3 uniform with name and values
	void setVec3(const std::string& name, float x, float y, float z) const {
		glUniform3f(glGetUniformLocation(ID, name.c_str()), x, y, z);
	}

	//Sets Vec3 uniform with name and vec3
	void setVec3(const std::string& name, const glm::vec3& value) const {
		glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, glm::value_ptr(value));
	}
};
#endif