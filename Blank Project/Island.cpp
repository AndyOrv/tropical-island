#include "Island.h"



Island::Island(OGLRenderer& rend, Camera& cam) 
{
	this->r = &rend;
	this->c = &cam;

	heightMap = new HeightMap(TEXTUREDIR"islandbrushed.png");
	Vector3 dimensions = heightMap->GetHeightmapSize();
	light = new Light(dimensions * Vector3(0.5f, 10.0f, 0.5f),
		Vector4(1, 1, 1, 1), dimensions.x * 100.0f);

	shader = new Shader("TerrainVertex.glsl", "TerrainFragment.glsl");
	lightShader = new Shader("PerPixelVertex.glsl", "PerPixelFragment.glsl");

	terrainTex1 = SOIL_load_OGL_texture(TEXTUREDIR"rock.JPG",
		SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	terrainTex2 = SOIL_load_OGL_texture(TEXTUREDIR"grass.JPG",
		SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	terrainTex3 = SOIL_load_OGL_texture(TEXTUREDIR"Sand Seamless.JPG",
		SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	
	if (!lightShader->LoadSuccess() || !shader->LoadSuccess()) { return; }
	if (!terrainTex1 || !terrainTex2 || !terrainTex3) { return; }

	r->SetTextureRepeating(terrainTex1, true);
	r->SetTextureRepeating(terrainTex2, true);
	r->SetTextureRepeating(terrainTex3, true);
}

Island::~Island()
{
	delete heightMap;
	delete shader;
	delete lightShader;
}

void Island::Draw(float dt, GLuint shadowTex)
{
	r->BindShader(shader);
	r->UpdateShaderMatrices();
	glUniform1i(glGetUniformLocation(lightShader->GetProgram(), "diffuseTex"), 0);
	glUniform1i(glGetUniformLocation(shader->GetProgram(), "rockTex"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, terrainTex1);
	glUniform1i(glGetUniformLocation(shader->GetProgram(), "grassTex"), 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, terrainTex2);
	glUniform1i(glGetUniformLocation(shader->GetProgram(), "sandTex"), 2);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, terrainTex3);

	glUniform3fv(glGetUniformLocation(shader->GetProgram(), "cameraPos"), 1, (float*)&c->GetPosition());

	r->modelMatrix.ToIdentity();
	r->textureMatrix.ToIdentity();

	r->UpdateShaderMatrices();
	r->SetShaderLight(*light);


	heightMap->Draw();
}
