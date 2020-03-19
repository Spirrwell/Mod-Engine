#ifndef RENDERVIEW_HPP
#define RENDERVIEW_HPP

#include "glm/glm.hpp"

struct RenderView
{
	glm::mat4 viewMatrix;
	glm::mat4 projectionMatrix;
};

#endif // RENDERVIEW_HPP