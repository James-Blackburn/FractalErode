#ifndef TREE_HPP_INCLUDED
#define TREE_HPP_INCLUDED

#include <cstdint>
#include <vector>
#include "vec3.hpp"

class InstancedTree {
private:
	unsigned int VAO = 0;
	unsigned int EBO = 0;
	unsigned int IBO = 0;
	int instanceCount = 0;
public:
	void init(float, float);
	void setPositions(std::vector<Vec3>& offsets);
	void render();
	void clean();
};

#endif