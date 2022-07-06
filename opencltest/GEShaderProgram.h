#pragma once
#include "GEShader.h"
#include <vector>
#include <memory>
#include "glm.hpp"

class GEShaderProgram
{
public:
	GEShaderProgram(void);
	~GEShaderProgram(void);

	void AttachShader(std::shared_ptr<GEShader> shader);
	void DetachShader(std::shared_ptr<GEShader> shader);

	void SetUniform_Mat4(std::string name, glm::mat4& val);
	void SetUniform_Vec3(std::string name, glm::vec3& val);
	void SetUniform_Vec2(std::string name, glm::vec2& val);
	void SetUniform_IVec2(std::string name, glm::ivec2& val);
	void SetUniform_Float(std::string name, float val);

	GLuint ProgramID(){ return programID; }

	void Use();
	void Link();

	
private:
	GLuint programID;
	std::vector<std::shared_ptr<GEShader>> shaderList;
};

