#ifndef RENDERLIST_HPP
#define RENDERLIST_HPP

#include "glm/glm.hpp"
#include "mesh.hpp"
#include <set>

struct RenderInfo
{
	Mesh *mesh = nullptr;
	glm::mat4 modelMat = {};

	// These are used by std::multiset, and are not meant for general comparison
	friend bool operator==( const RenderInfo &lhs, const RenderInfo &rhs ) { return lhs.mesh->GetMeshIndex() == rhs.mesh->GetMeshIndex(); }
	friend bool operator< ( const RenderInfo &lhs, const RenderInfo &rhs ) { return lhs.mesh->GetMeshIndex() < rhs.mesh->GetMeshIndex(); }
};

using RenderList = std::multiset< RenderInfo >;

#endif // RENDERLIST_HPP