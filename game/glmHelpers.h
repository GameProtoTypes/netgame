#pragma once


#include "glm.hpp"
#include <glm.hpp>


void SetTranslation(glm::mat4& matrix, glm::vec3& translation)
{
	matrix[3] = glm::vec4(translation, 1.0f);
}


void SetScale_DiscardRotation(glm::mat4& matrix, glm::vec3& scale)
{
	matrix[0][0] = scale.x;
	matrix[1][1] = scale.y;
	matrix[2][2] = scale.z;
}