#ifndef __MESH_H__
#define __MESH_H__
#pragma once

#include "mathes.hpp"

struct Mesh
{
	size_t numVertex;
	size_t numFaces;
    
	std::vector<vec3f> vertexs;
	std::vector<vec3f> vertexTextures;
	std::vector<vec3f> vertexsNormal;
	std::vector<vec3i> faceVertexIndex;
	std::vector<vec3i> faceTextureIndex;
	std::vector<vec3i> faceNormalIndex;
};

namespace ObjParser
{
    bool ParseMesh(const char*,Mesh*);
    bool ParseMesh(const char*, size_t, Mesh*);
};

#endif /// of __MESH_H__
