#include "GEShader.h"
#include <string>
#include <iostream>
#include <fstream>

GEShader::GEShader(GLenum shaderType, std::string filepath)
	: shaderType(shaderType)
{
	shaderID = glCreateShader(shaderType);


	if (!LoadSource(filepath))
	{
		std::cout << "Error Loading Source \"" << filepath << "\" for shader!";
		exit(EXIT_FAILURE);
	}
	const GLchar *source = sourceCode.c_str();
	glShaderSource(shaderID, 1, &source, NULL);

	glCompileShader(shaderID);
	
	//check errors....
	GLint status;
	glGetShaderiv(shaderID, GL_COMPILE_STATUS, &status);
	if (status == GL_FALSE)
	{
		std::cout << "ERROR Compiling shader" << std::endl;
		std::cout << sourceCode << std::endl;
	}
	std::cout << "shader " << shaderID << " created" << std::endl;
}


GEShader::~GEShader()
{
}


//TODO FIX!
bool GEShader::LoadSource(std::string filepath)
{

	
	std::ifstream fin(filepath);
	if (!fin.is_open()) 
		return false;

	std::string line;
	sourceCode.clear();
	while (std::getline(fin, line))
	{
		sourceCode.append(line);
		sourceCode.push_back('\n');
	}

	fin.close();

	return true;

}