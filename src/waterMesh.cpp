#include "waterMesh.hpp"
#include "vec3.hpp"
#include <glad/glad.h>
#include <omp.h>

void WaterMesh::generate(uint32_t _terrainHeightVBO, float* hmapWater) {
    terrainHeightVBO = _terrainHeightVBO;
    HeightMesh::generate(hmapWater);
}

WaterMesh::~WaterMesh() {
    clean();
}

void WaterMesh::sendGPU() {
    // send mesh data to GPU
    // generate buffers first time
    if (!createdOnGPU) {
        createdOnGPU = true;

        // generate buffers
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, 0, (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)(xWidth * zWidth * sizeof(float)));

        glBindBuffer(GL_ARRAY_BUFFER, terrainHeightVBO);
        glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 0, (void*)0);
        glEnableVertexAttribArray(2);

        glBindVertexArray(0);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, numIndices * sizeof(uint32_t), indices, GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }

    // buffer orphaning
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, xWidth * zWidth * 4 * sizeof(float), NULL, GL_DYNAMIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, xWidth * zWidth * sizeof(float), heights);
    glBufferSubData(GL_ARRAY_BUFFER, xWidth * zWidth * sizeof(float), xWidth * zWidth * 3 * sizeof(float), normals);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    needSendGPU = false;
};

void WaterMesh::clean() {
    terrainHeightVBO = 0;
    HeightMesh::clean();
}
