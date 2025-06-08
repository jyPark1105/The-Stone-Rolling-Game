#ifndef OBJECTMESH_H
#define OBJECTMESH_H

#include <vector>
#include <glm/glm.hpp>

struct ObjectMesh
{
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec3> normals;
	std::vector<glm::vec2> textureCoordinates;
	std::vector<glm::vec3> tangents;
	std::vector<glm::vec3> bitangents;
	std::vector<glm::ivec4> boneIDs;
	std::vector<glm::vec4> weights;

	std::vector<unsigned int> indices;
};

#endif