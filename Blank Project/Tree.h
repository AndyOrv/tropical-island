#pragma once

#include "../nclgl/Mesh.h"
#include "../nclgl/Camera.h"
#include "../nclgl/SceneNode.h"
#include "../nclgl/MeshMaterial.h"
#include "../nclgl/Heightmap.h"
#include "../nclgl/Light.h"

class Tree : public SceneNode
{
public:
	Tree(OGLRenderer& rend);
	~Tree();

	virtual void Draw(float dt, GLuint shadowTex);

protected:
	OGLRenderer* r;
	Shader* shader;
	Mesh* mesh;
	MeshMaterial* material;

	HeightMap* terrainMap;
	vector<GLuint> matTextures;

	const float heightOffset = 100;
	int treeCount;
	int treePositions[1000];
	int treeRotations[1000];
};
