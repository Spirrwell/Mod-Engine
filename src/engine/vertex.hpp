#ifndef VERTEX_HPP
#define VERTEX_HPP

#include "glm/glm.hpp"

struct Vertex
{
	glm::vec3 position;
	glm::vec2 texCoord;
	glm::vec3 normal;
	glm::vec3 color = glm::vec3( 1.0f, 1.0f, 1.0f );
};

#endif // VERTEX_HPP