#ifndef TERRAIN_MESH_HPP_INCLUDED
#define TERRAIN_MESH_HPP_INCLUDED

#include "heightMesh.hpp"

class TerrainMesh : public HeightMesh
{
private:
	float* altitude = nullptr;
	unsigned int altitudeVBO = 0;

public:
	~TerrainMesh();
	void generate(float*, float*);
	void sendGPU();
	void clean();
	inline unsigned int getVBO() const { return VBO; }
	void updateAltitude();
};

#endif
