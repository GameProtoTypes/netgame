#pragma once


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

