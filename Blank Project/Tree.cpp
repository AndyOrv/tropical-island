#include "Tree.h"
#include <random>

Tree::Tree(OGLRenderer& rend)
{
	//initialize
	this->r = &rend;
	shader = new Shader("TexturedVertex.glsl", "TexturedFragment.glsl");
	mesh = Mesh::LoadFromMeshFile("CoconutTree.msh");
	material = new MeshMaterial("CoconutTree.mat");
	treeCount = 100;
	terrainMap = new HeightMap(TEXTUREDIR"islandbrushed.png");


	for (int i = 0; i < mesh->GetSubMeshCount(); ++i)
	{
		const  MeshMaterialEntry* matEntry = material->GetMaterialForLayer(i);
		const  string* filename = nullptr;
		matEntry->GetEntry("Diffuse", &filename);
		string  path = TEXTUREDIR + *filename;
		GLuint  texID = SOIL_load_OGL_texture(path.c_str(), SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y);
		matTextures.emplace_back(texID);
	}


	int totalVertices = terrainMap->GetImageWidth() * terrainMap->GetImageWidth();
	std::cout << terrainMap->vertices->Length() << std::endl;
	//int totalVertices = 900;
	std::cout << totalVertices << std::endl;

	// random number picker for verites
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> pickVertice(1, totalVertices);
	std::uniform_int_distribution<> pickRotation(1, 360);


	for (int i = 0; i < treeCount; i++)
	{
		treePositions[i] = pickVertice(gen);
		treeRotations[i] = pickRotation(gen);

		std::cout << treePositions[i] << std::endl;

		while (terrainMap->vertices[treePositions[i]].y < 215) { treePositions[i] = pickVertice(gen); }
	}
}

Tree::~Tree()
{
	delete shader;
	delete mesh;
	delete material;
	delete terrainMap;
}

void Tree::Draw(float dt, GLuint shadowTex)
{
	Matrix4 temp = r->modelMatrix;

	r->BindShader(shader);

	for (int i = 0; i < treeCount; i++)
	{
		glUniform1i(glGetUniformLocation(shader->GetProgram(), "diffuseTex"), 0);
		//std::cout << treePositions[i] << std::endl;
		r->modelMatrix =
			Matrix4::Translation(Vector3(
				terrainMap->vertices[treePositions[i]].x,
				terrainMap->vertices[treePositions[i]].y - heightOffset,
				terrainMap->vertices[treePositions[i]].z))
			* Matrix4::Scale(Vector3(16, 16, 16))
			* Matrix4::Rotation(treeRotations[i], Vector3(0.0f, 1.0f, 0.0f));

		r->UpdateShaderMatrices();

		for (int i = 0; i < mesh->GetSubMeshCount(); ++i)
		{
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, matTextures[i]);
			mesh->DrawSubMesh(i);
		}

	}
	r->modelMatrix = temp;
}