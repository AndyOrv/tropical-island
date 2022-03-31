#include "Renderer.h"
#include "../nclgl/camera.h"
#include "../nclgl/HeightMap.h"
#include "../nclgl/Light.h"
#include "../nclgl/MeshAnimation.h"
#include "../nclgl/MeshMaterial.h"
#include  <algorithm>    

const int POST_PASSES = 10;
constexpr auto SHADOWSIZE = 2048;
constexpr auto LIGHT_NUM = 30;

Renderer::Renderer(Window& parent) : OGLRenderer(parent) {
	quad = Mesh::GenerateQuad();
	sphere = Mesh::LoadFromMeshFile("Sphere.msh");
	heightMap = new HeightMap(TEXTUREDIR"islandbrushed.png");
	
	camera = new Camera(-40, 270, Vector3());
	Vector3 dimensions = heightMap->GetHeightmapSize();
	camera->SetPosition(dimensions * Vector3(0.5, 2, 0.5));

	root = new SceneNode();
	mapNode = new Island(*this, *camera);
	root->AddChild(mapNode);
	root->AddChild(new Tree(*this));


	waterTex = SOIL_load_OGL_texture(
		TEXTUREDIR"water.TGA", SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);

	cubeMap = SOIL_load_OGL_cubemap(
		//TEXTUREDIR"rusted_west.jpg", TEXTUREDIR"rusted_east.jpg",
		//TEXTUREDIR"rusted_up.jpg", TEXTUREDIR"rusted_down.jpg",
		//TEXTUREDIR"rusted_south.jpg", TEXTUREDIR"rusted_north.jpg",

		TEXTUREDIR "west.png", TEXTUREDIR "east.png",
		TEXTUREDIR "up.png", TEXTUREDIR "down.png",
		TEXTUREDIR "south.png", TEXTUREDIR "north.png",
		SOIL_LOAD_RGB, SOIL_CREATE_NEW_ID, 0);

	if (!cubeMap || !waterTex) {
		return;
	}

	SetTextureRepeating(waterTex, true);

	reflectShader = new Shader(
		"reflectVertex.glsl", "reflectFragment.glsl");
	skyboxShader = new Shader(
		"skyboxVertex.glsl", "skyboxFragment.glsl");
	lightShader = new Shader(
		"PerPixelVertex.glsl", "PerPixelFragment.glsl");

	if (!reflectShader->LoadSuccess() ||
		!skyboxShader->LoadSuccess() ||
		!lightShader->LoadSuccess()) {
		return;
	}

	shadowShader = new Shader("shadowVert.glsl", "shadowFrag.glsl");

	glGenTextures(1, &shadowTex);
	glBindTexture(GL_TEXTURE_2D, shadowTex);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
		SHADOWSIZE, SHADOWSIZE, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

	glBindTexture(GL_TEXTURE_2D, 0);

	glGenFramebuffers(1, &shadowFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
		GL_TEXTURE_2D, shadowTex, 0);
	glDrawBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	////////////////////////////////////

	manShader = new Shader("SkinningVertex.glsl", "texturedFragment.glsl");

	if (!manShader->LoadSuccess()) {
		return;
	}


	manMesh = Mesh::LoadFromMeshFile("Role_T.msh");
	manAnim = new MeshAnimation("Role_T.anm");
	manMaterial = new MeshMaterial("Role_T.mat");

	for (int i = 0; i < manMesh->GetSubMeshCount(); ++i) {
		const MeshMaterialEntry* matEntry = manMaterial->GetMaterialForLayer(i);

		const string* filename = nullptr;
		matEntry->GetEntry("Diffuse", &filename);
		string path = TEXTUREDIR + *filename;

		GLuint texID = SOIL_load_OGL_texture(path.c_str(), SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y);
		matTextures.emplace_back(texID);
	}
	currentFrame = 0;
	frameTime = 0.0f;


	//////////////////////////////////////


	light = new Light(dimensions * Vector3(0.5f, 10.0f, 0.5f),
		Vector4(1, 1, 1, 1), dimensions.x * 100.0f);

	projMatrix = Matrix4::Perspective(1.0f, 10000.0f,
		(float)width / (float)height, 45.0f);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glEnable(GL_CULL_FACE);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glCullFace(GL_BACK);
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	waterRotate = 0.0f;
	waterCycle = 0.0f;
	init = true;
}

Renderer ::~Renderer(void) {
	delete heightMap;
	delete camera;

	delete root;
	delete quad;
	delete manMesh;
	delete manAnim;
	delete manMaterial;

	delete skyboxShader;
	delete manShader;
	delete pointlightShader;
	delete processShader;
	delete combinedShader;
	delete shadowShader;

	delete reflectShader;
	delete skyboxShader;
	delete lightShader;
	delete light;
}

void Renderer::UpdateScene(float dt) {
	camera->UpdateCamera(dt);
	viewMatrix = camera->BuildViewMatrix();

	frameFrustum.FromMatrix(projMatrix * viewMatrix);

	waterRotate += dt * 0.25f; //2 degrees a second
	waterCycle += dt * 0.05f; //10 units a second

	frameTime -= dt;
	while (frameTime < 0.0f) {
		currentFrame = (currentFrame + 1) % manAnim->GetFrameCount();
		frameTime += 1.0f / manAnim->GetFrameRate();
	}

	root->Update(dt);
}

void Renderer::DrawSkybox() {
	glDepthMask(GL_FALSE);

	BindShader(skyboxShader);
	UpdateShaderMatrices();

	quad->Draw();

	glDepthMask(GL_TRUE);
}

void Renderer::DrawMan()
{

	BindShader(manShader);
	glUniform1i(glGetUniformLocation(manShader->GetProgram(), "diffuseTex"), 0);

	modelMatrix =
		Matrix4::Translation(Vector3(
			heightMap->vertices[174102].x,
			heightMap->vertices[174102].y,
			heightMap->vertices[174102].z))
		* Matrix4::Scale(Vector3(50, 50, 50))
		* Matrix4::Rotation(0, Vector3(0.0f, 1.0f, 0.0f));

	UpdateShaderMatrices();

	vector <Matrix4 > frameMatrices;

	const Matrix4* invBindPose = manMesh->GetInverseBindPose();
	const Matrix4* frameData = manAnim->GetJointData(currentFrame);

	for (unsigned int i = 0; i < manMesh->GetJointCount(); ++i) {
		frameMatrices.emplace_back(frameData[i] * invBindPose[i]);
	}

	int j = glGetUniformLocation(manShader->GetProgram(), "joints");
	glUniformMatrix4fv(j, frameMatrices.size(), false, (float*)frameMatrices.data());

	for (int i = 0; i < manMesh->GetSubMeshCount(); ++i) {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, matTextures[i]);
		manMesh->DrawSubMesh(i);
	}
}

void Renderer::DrawWater() {
	BindShader(reflectShader);

	glUniform3fv(glGetUniformLocation(reflectShader->GetProgram(),
		"cameraPos"), 1, (float*)&camera->GetPosition());

	glUniform1i(glGetUniformLocation(reflectShader->GetProgram(), "diffuseTex"), 0);
	glUniform1i(glGetUniformLocation(reflectShader->GetProgram(), "cubeTex"), 2);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, waterTex);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMap);

	Vector3 hSize = heightMap->GetHeightmapSize();

	modelMatrix =
		Matrix4::Translation(hSize * 0.30f) *
		Matrix4::Scale(hSize) *
		Matrix4::Rotation(270, Vector3(1, 0, 0));


	textureMatrix =
		Matrix4::Translation(Vector3(waterCycle, 0.0f, waterCycle)) *
		Matrix4::Scale(Vector3(10, 10, 10)) *
		Matrix4::Rotation(waterRotate, Vector3(0, 0, 1));

	UpdateShaderMatrices();
	// SetShaderLight (* light);
	quad->Draw();
}


void Renderer::BuildNodeLists(SceneNode* from)
{
	if (frameFrustum.InsideFrustum(*from)) {
		Vector3 dir = from->GetWorldTransform().GetPositionVector() - camera->GetPosition();
		from->SetCameraDistance(Vector3::Dot(dir, dir));

		if (from->GetColour().w < 1.0f) {
			transparentNodeList.push_back(from);
		}
		else {
			nodeList.push_back(from);
		}
	}

	for (vector <SceneNode*>::const_iterator i = from->GetChildIteratorStart(); i != from->GetChildIteratorEnd(); ++i) {
		BuildNodeLists((*i));
	}

}

void Renderer::SortNodeLists()
{
	std::sort(transparentNodeList.rbegin(),
		transparentNodeList.rend(),
		SceneNode::CompareByCameraDistance);
	std::sort(nodeList.begin(),
		nodeList.end(),
		SceneNode::CompareByCameraDistance);
}


void Renderer::DrawNodes()
{	
	for (vector <SceneNode*>::const_iterator it = root->GetChildIteratorStart(); it != root->GetChildIteratorEnd(); ++it)
	{
		(*it)->Draw(0,0);
	}
}

void Renderer::RenderScene() {
	BuildNodeLists(root);
	SortNodeLists();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//DrawPointLights();
	DrawSkybox();
	DrawWater();
	DrawMan();
	
	DrawNodes();
	ClearNodeLists();
}

void Renderer::ClearNodeLists()
{
	transparentNodeList.clear();
	nodeList.clear();
}

void Renderer::DrawPostProcess() {
	glBindFramebuffer(GL_FRAMEBUFFER, processFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bufferColourTex[1], 0);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	BindShader(processShader);
	modelMatrix.ToIdentity();
	viewMatrix.ToIdentity();
	projMatrix.ToIdentity();
	UpdateShaderMatrices();

	glDisable(GL_DEPTH_TEST);
	glActiveTexture(GL_TEXTURE0);
	glUniform1i(glGetUniformLocation(processShader->GetProgram(), "sceneTex"), 0);

	for (int i = 0; i < POST_PASSES; ++i) {
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bufferColourTex[1], 0);
		glUniform1i(glGetUniformLocation(processShader->GetProgram(), "isVertical"), 0);
		glBindTexture(GL_TEXTURE_2D, bufferColourTex[0]);
		quad->Draw();
		//Now to swap the colour buffers , and do the second blur pass
		glUniform1i(glGetUniformLocation(processShader->GetProgram(), "isVertical"), 1);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bufferColourTex[0], 0);
		glBindTexture(GL_TEXTURE_2D, bufferColourTex[1]);
		quad->Draw();
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glEnable(GL_DEPTH_TEST);
}

void Renderer::PresentScene() {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	BindShader(combinedShader);
	modelMatrix.ToIdentity();
	viewMatrix.ToIdentity();
	projMatrix.ToIdentity();
	UpdateShaderMatrices();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, bufferColourTex[0]);
	glUniform1i(glGetUniformLocation(combinedShader->GetProgram(), "diffuseTex"), 0);
	quad->Draw();
}

void Renderer::DrawShadowScene() {
	glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
	glClear(GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, SHADOWSIZE, SHADOWSIZE);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

	BindShader(shadowShader);
}

void Renderer::DrawPointLights() {
	glBindFramebuffer(GL_FRAMEBUFFER, pointLightFBO);
	BindShader(pointlightShader);

	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT);
	glBlendFunc(GL_ONE, GL_ONE);
	glCullFace(GL_FRONT);
	glDepthFunc(GL_ALWAYS);
	glDepthMask(GL_FALSE);

	glUniform1i(glGetUniformLocation(
		pointlightShader->GetProgram(), "depthTex"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, bufferDepthTex);

	glUniform1i(glGetUniformLocation(
		pointlightShader->GetProgram(), "normTex"), 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, bufferNormalTex);

	glUniform3fv(glGetUniformLocation(pointlightShader->GetProgram(),
		"cameraPos"), 1, (float*)&camera->GetPosition());

	glUniform2f(glGetUniformLocation(pointlightShader->GetProgram(),
		"pixelSize"), 1.0f / width, 1.0f / height);

	Matrix4 invViewProj = (projMatrix * viewMatrix).Inverse();
	glUniformMatrix4fv(glGetUniformLocation(
		pointlightShader->GetProgram(), "inverseProjView"),
		1, false, invViewProj.values);
	modelMatrix.ToIdentity();
	UpdateShaderMatrices();
	for (int i = 0; i < LIGHT_NUM; ++i) {
		Light& l = pointLights[i];
		SetShaderLight(l);
		sphere->Draw();

	}

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glCullFace(GL_BACK);
	glDepthFunc(GL_LEQUAL);

	glDepthMask(GL_TRUE);

	glClearColor(0.2f, 0.2f, 0.2f, 1);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

}
