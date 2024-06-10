#include "tree.hpp"
#include <glad/glad.h>

void InstancedTree::init(float sizeH, float sizeV) {
	const float vertices[] = {
		-sizeH / 2.0f, +0.0f, 0.0f,		0.0f, 1.0f,
		+sizeH / 2.0f, +0.0f, 0.0f,		1.0f, 1.0f,
		+sizeH / 2.0f, sizeV, 0.0f,		1.0f, 0.0f,
		+sizeH / 2.0f, sizeV, 0.0f,		1.0f, 0.0f,
		-sizeH / 2.0f, sizeV, 0.0f,		0.0f, 0.0f,
		-sizeH / 2.0f, +0.0f, 0.0f,		0.0f, 1.0f,

		-sizeH / 2.0f, +0.0f, 0.0f,		0.0f, 1.0f,
		+sizeH / 2.0f, sizeV, 0.0f,		1.0f, 0.0f,
		+sizeH / 2.0f, +0.0f, 0.0f,		1.0f, 1.0f,
		+sizeH / 2.0f, sizeV, 0.0f,		1.0f, 0.0f,
		-sizeH / 2.0f, +0.0f, 0.0f,		0.0f, 1.0f,
		-sizeH / 2.0f, sizeV, 0.0f,		0.0f, 0.0f,

		0.0f, +0.0f, -sizeH / 2.0f,		0.0f, 1.0f,
		0.0f, +0.0f, +sizeH / 2.0f,		1.0f, 1.0f,
		0.0f, sizeV, +sizeH / 2.0f,		1.0f, 0.0f,
		0.0f, sizeV, +sizeH / 2.0f,		1.0f, 0.0f,
		0.0f, sizeV, -sizeH / 2.0f,		0.0f, 0.0f,
		0.0f, +0.0f, -sizeH / 2.0f,		0.0f, 1.0f,

		0.0f, +0.0f, -sizeH / 2.0f,		0.0f, 1.0f,
		0.0f, sizeV, +sizeH / 2.0f,		1.0f, 0.0f,
		0.0f, +0.0f, +sizeH / 2.0f,		1.0f, 1.0f,
		0.0f, sizeV, +sizeH / 2.0f,		1.0f, 0.0f,
		0.0f, +0.0f, -sizeH / 2.0f,		0.0f, 1.0f,
		0.0f, sizeV, -sizeH / 2.0f,		0.0f, 0.0f,
	};

	unsigned int VBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(VAO);
	
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &IBO);
	glBindBuffer(GL_ARRAY_BUFFER, IBO);
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glVertexAttribDivisor(3, 1);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	glBindVertexArray(0);

	glDeleteBuffers(1, &VBO);
}

void InstancedTree::setPositions(std::vector<Vec3>& offsets) {
	instanceCount = offsets.size();

	glBindBuffer(GL_ARRAY_BUFFER, IBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vec3) * offsets.size(), offsets.data(), GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void InstancedTree::render() {
	glBindVertexArray(VAO);
	glDrawArraysInstanced(GL_TRIANGLES, 0, 24, instanceCount);
	glBindVertexArray(0);
}

void InstancedTree::clean() {
	glDeleteBuffers(1, &IBO);
	glDeleteVertexArrays(1, &VAO);
	instanceCount = 0;
}