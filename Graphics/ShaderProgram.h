#pragma once

#ifndef SHADER_H
#define SHADER_H

#include <GL/glew.h>
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include<glm.hpp>
#include "ShaderObj.h"

//Creates a Shader program with stored fragmentAttributes
class ShaderProgram
{
public:
	unsigned int ID;
	//Activate Shader
	//Utility uniform functions
	ShaderProgram( VertexShader &vertexShader,  FragmentShader &fragmentShader)
	{
		// Compiles both shaders and sets there GLuint ids to later be disposed
		vertexShader.id = vertexShader.compileShader();
		fragmentShader.id = fragmentShader.compileShader();
		// Create our shader program
		ID = glCreateProgram();
		//Attach our shaders
		glAttachShader(ID, vertexShader.id);
		glAttachShader(ID, fragmentShader.id);
		glLinkProgram(ID);
		//Check if linking Worked
		GLint success;
		char infoLog[512];
		glGetProgramiv(ID, GL_LINK_STATUS, &success);
		if (!success) {
			glGetProgramInfoLog(ID, 512, NULL, infoLog);
			std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
		}
		glBindFragDataLocation(ID, 0, "outColor");

	}
	ShaderProgram() {

	}
	//Set ShaderProgram as current glfw shader
	void use()
	{
		glUseProgram(ID);
	}
	//Disposes program
	void Delete() {
		glDeleteProgram(ID);
	}
	//Sets bool uniform with name, and value
	void setBool(const std::string& name, bool value) const
	{
		glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
	}
	//Sets int uniform with name, and value
	void setInt(const std::string& name, int value) const
	{
		glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
	}
	//Sets float uniform with name, and value
	void setFloat(const std::string& name, float value) const
	{
		glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
	}
	//Sets Matrix 4 uniform with name, and value
	void setMat4(const std::string& name, const GLfloat* mat) const {
		glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, mat);
	}
};
#endif 