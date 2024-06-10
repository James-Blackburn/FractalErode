#ifndef WATER_MESH_HPP_INCLUDED
#define WATER_MESH_HPP_INCLUDED

#include "heightMesh.hpp"

class WaterMesh : public HeightMesh {
private:
	unsigned int terrainHeightVBO = 0;

public:
	~WaterMesh();
	void generate(uint32_t, float*);
	void sendGPU();
	void clean();
};

#endif
