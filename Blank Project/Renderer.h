sssssssssssss#pragma once
#include "../nclgl/OGLRenderer.h"
#include "../nclgl/SceneNode.h"
#include "../nclgl/Frustum.h"
#include "../nclgl/Heightmap.h"
#include "../nclgl/Mesh.h"
#include "Tree.h"
#include "Island.h"
#include "../nclgl/Mesh.h"
#include "../nclgl/Camera.h"
#include "../nclgl/SceneNode.h"
#include "../nclgl/MeshMaterial.h"
#include "../nclgl/Heightmap.h"
#include "../nclgl/Light.h"

class HeightMap;
class Camera;
class Mesh;
class MeshAnimation;
class MeshMaterial;

class Renderer : public OGLRenderer {
public:
	Renderer(Window& parent);
	~Renderer(void);
	void RenderScene() override;
	void UpdateScene(float dt) override;

protected:
	void DrawSkybox();
	void DrawWater();
	void DrawHeightmap();
	void DrawMan();

	void BuildNodeLists(SceneNode* from);
	void SortNodeLists();
	void ClearNodeLists();
	void DrawNodes();
	void DrawNode(SceneNode* n);

	void DrawShadowScene();
	void DrawPointLights();

	void DrawPostProcess();
	void PresentScene();

	SceneNode* root;
	Island* mapNode;

	Light* light;
	Shader* lightShader;
	Shader* reflectShader;

	Shader* skyboxShader;
	GLuint cubeMap;
	
	float waterRotate;
	float waterCycle;
	GLuint waterTex;

	//terrain
	HeightMap* heightMap;
	vector <GLuint > matTextures;

	Mesh* quad;
	Mesh* sphere;
	GLuint texture;

	//animation
	Mesh* manMesh;
	Shader* manShader;
	MeshAnimation* manAnim;
	MeshMaterial* manMaterial;
	vector <GLuint > manMatTextures;

	int currentFrame;
	float frameTime;


	Frustum frameFrustum;
	Camera* camera;

	Light* pointLights;
	Shader* pointlightShader;
	Shader* processShader;
	Shader* combinedShader;
	GLuint bufferFBO;
	GLuint processFBO;
	GLuint bufferColourTex[2];
	GLuint bufferDepthTex;
	GLuint pointLightFBO;
	GLuint bufferNormalTex;

	GLuint shadowTex;
	GLuint shadowFBO;
	Shader* shadowShader;

	GLuint bufferFBO2;
	GLuint bufferColourTex2;
	GLuint bufferDepthTex2;

	vector <SceneNode*> transparentNodeList;
	vector <SceneNode*> nodeList;
};