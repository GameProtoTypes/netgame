#include "GEShaderProgram.h"
#include "glew.h"
#include "glfw3native.h"

#include <iostream>

GEShaderProgram::GEShaderProgram(void) 
{
	programID = glCreateProgram();
}


GEShaderProgram::~GEShaderProgram(void)
{

	for(auto sh : shaderList)
		DetachShader(sh);

	glDeleteProgram(programID);
}

void GEShaderProgram::Link(void)
{
	glLinkProgram(programID);

	GLint success;
	glGetProgramiv(programID, GL_LINK_STATUS, &success);

	if (!success)
	{

		std::cout << "Error Linking shader program " << programID << "." << std::endl;
		GLint maxLength = 0;
		glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &maxLength);

		// The maxLength includes the NULL character
		std::vector<GLchar> infoLog(maxLength);
		glGetProgramInfoLog(programID, maxLength, &maxLength, &infoLog[0]);

		for (int i = 0; i < infoLog.size(); i++)
		{
			std::cout << infoLog[i];
		}

	}
}

void GEShaderProgram::Use(void)
{
	glUseProgram(programID);
}

void GEShaderProgram::AttachShader(GEShader* shader)
{
	glAttachShader(programID, shader->shaderID);

	shaderList.push_back(shader);
}

void GEShaderProgram::DetachShader(GEShader* shader)
{
	for(unsigned int i = 0; i < shaderList.size(); i++)
	{
		if(shaderList[i] == shader)
		{
			glDetachShader(programID, shader->shaderID);
			break;
		}
	}
}

void GEShaderProgram::SetUniform_Mat4(std::string name, glm::mat4& val)
{
	GLint location = glGetUniformLocation(programID,
		name.c_str());

	if (location >= 0)
	{
		glUniformMatrix4fv(location, 1, GL_FALSE,
			&val[0][0]);
	}
}

void GEShaderProgram::SetUniform_Vec3(std::string name, glm::vec3& val)
{
	GLint location = glGetUniformLocation(programID,
		name.c_str());

	if (location >= 0)
	{
		glUniform3f(location, val.x,val.y,val.z);

	}

}
