#pragma once
#include "GEShader.h"
#include <vector>

class GEShaderProgram
{
public:
	GEShaderProgram(void);
	~GEShaderProgram(void);

	void AttachShader(GEShader* shader);
	void DetachShader(GEShader* shader);

	GLuint ProgramID(){ return programID; }

	void Use();
	void Link();

	
private:
	GLuint programID;
	std::vector<GEShader*> shaderList;
};

