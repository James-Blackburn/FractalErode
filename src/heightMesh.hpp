#ifndef HEIGHT_MESH_HPP_INCLUDED
#define HEIGHT_MESH_HPP_INCLUDED

#include <atomic>
#include "vec3.hpp"

class HeightMesh {
protected:
	unsigned int VAO = 0;
	unsigned int EBO = 0;
	unsigned int VBO = 0;

	float* heights = nullptr;
	Vec3* normals = nullptr;
	unsigned int* indices = nullptr;

	float cellSize = 1.0f;
	int xWidth = 256;
	int zWidth = 256;

	bool createdOnGPU = false;

public:
	unsigned int numIndices = 0;
	std::atomic<bool> generated = false;
	std::atomic<bool> needSendGPU = false;

	HeightMesh() = default;
	HeightMesh(float*, float, int, int);
	~HeightMesh();

	void init(float, int, int);
	void generate(float*);
	void sendGPU();
	void render();
	void clean();
	inline Vec3* getNormals() { return normals; }
};

#endif