#pragma once

#include "glew.h"
#include "glfw3native.h"


#include <string>

class GEShader 
{
public:
	GEShader(GLenum shaderType, std::string filepath);
	~GEShader();

	friend class GEShaderProgram;

private:

	bool LoadSource(std::string filepath);

	std::string sourceCode;

	GLuint shaderID;
	GLenum shaderType;
};

