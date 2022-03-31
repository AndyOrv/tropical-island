#pragma once

#include "../nclgl/Mesh.h"
#include "../nclgl/Camera.h"
#include "../nclgl/SceneNode.h"
#include "../nclgl/MeshMaterial.h"
#include "../nclgl/Heightmap.h"
#include "../nclgl/Light.h"


class Island : public SceneNode
{
public:
	Island(OGLRenderer& rend, Camera& cam);
	~Island(void);

	virtual void Draw(float dt, GLuint shadowTex) override;

	void setLight(Light* l) { light = l; };
	void SetShader(Shader* s) { shader = s; };

protected:
	OGLRenderer* r;
	Camera* c;

	HeightMap* heightMap;
	Shader* shader;
	Light* light;
	Shader* lightShader;
	GLuint terrainTex1;
	GLuint terrainTex2;
	GLuint terrainTex3;
};