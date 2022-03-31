#pragma once
#include <string>
#include "mesh.h"

class HeightMap : public Mesh {
	friend class Tree;
public:
	HeightMap(const std::string& name);
	~HeightMap(void) {};

	Vector3 GetHeightmapSize() const { return heightmapSize; }
	int GetImageWidth() const { return image_width; }

protected:
	Vector3 heightmapSize;
	int image_width;
};