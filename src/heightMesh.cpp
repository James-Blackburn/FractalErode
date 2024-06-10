#include "heightMesh.hpp"
#include "vec3.hpp"
#include <glad/glad.h>
#include <omp.h>

HeightMesh::HeightMesh(float* verts, float _cellSize, int _xWidth, int _zWidth){
    cellSize = _cellSize;
    xWidth = _xWidth;
    zWidth = _zWidth;
}

HeightMesh::~HeightMesh() {
	clean();
}

void HeightMesh::init(float _cellSize, int _xWidth, int _zWidth) {
    cellSize = _cellSize;
    xWidth = _xWidth;
    zWidth = _zWidth;
}

void HeightMesh::generate(float* hmap) {
    if (needSendGPU) {
        needSendGPU = false;
    }

    if (!generated) {
        numIndices = (xWidth - 1) * (zWidth - 1) * 6;

        heights = new float[xWidth * zWidth];
        normals = new Vec3[xWidth * zWidth];
        indices = new uint32_t[numIndices];

        // create faces with indices
#pragma omp parallel for
        for (int z = 0; z < zWidth - 1; z++) {
            for (int x = 0; x < xWidth - 1; x++) {
                const int index = ((z * (xWidth - 1)) + x) * 6;
                indices[index] = (z * xWidth) + x;
                indices[index + 1] = ((z + 1) * xWidth) + x;
                indices[index + 2] = ((z + 1) * xWidth) + x + 1;
                indices[index + 3] = (z * xWidth) + x;
                indices[index + 4] = ((z + 1) * xWidth) + x + 1;
                indices[index + 5] = (z * xWidth) + x + 1;
            }
        }

        // iterate though vertices
#pragma omp parallel for
        for (int z = 0; z < zWidth; z++) {
            for (int x = 0; x < xWidth; x++) {
                const int index = z * xWidth + x;
                // set height according to heightmap elevation
                heights[index] = hmap[index];
                // set normals as zero for now
                normals[index].x = 0.0f;
                normals[index].y = 0.0f;
                normals[index].z = 0.0f;
            }
        }
    }
    else {
        // iterate though vertices, only set the updated y position this time
#pragma omp parallel for
        for (int z = 0; z < zWidth; z++) {
            for (int x = 0; x < xWidth; x++) {
                const int index = z * xWidth + x;
                // set vertice positions according to heightmap elevation
                heights[index] = hmap[index];
                // set normals as zero for now
                normals[index].x = 0.0f;
                normals[index].y = 0.0f;
                normals[index].z = 0.0f;
            }
        }
    }

    // generate smooth normals per 2-face cells
#pragma omp parallel for
    for (int z = 1; z < zWidth - 2; z ++) {
        for (int x = 1; x < xWidth - 2; x ++) {
            const int index = ((z * (xWidth - 1)) + x) * 6;
            const int in0 = indices[index]; // x,z
            const int in1 = indices[index + 1]; // x,z+1
            const int in2 = indices[index + 2]; // x+1,z+1
            const int in3 = indices[index + 5]; // x+1,z

            const Vec3 v0{ x, heights[z * xWidth + x], z };
            const Vec3 v1{ x, heights[(z+1) * xWidth + x], z+1 };
            const Vec3 v2{ x+1, heights[(z+1) * xWidth + x+1], z+1 };
            const Vec3 v3{ x+1, heights[z * xWidth + x+1], z };

            const Vec3 normal1 = cross(v1 - v0, v2 - v0);
            const Vec3 normal2 = cross(v2 - v0, v3 - v0);
            const Vec3 normal3 = normal1 + normal2;

            // Data race
            // not urgent to fix as normal precision is not essential
            normals[in0] += normal3;
            normals[in1] += normal1;
            normals[in2] += normal3;
            normals[in3] += normal2;
        }
    }

    generated = true;
    needSendGPU = true;
}

void HeightMesh::sendGPU() {
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
        glBindVertexArray(0);

        // send indices to GPU
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, numIndices * sizeof(uint32_t), indices, GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }

    // update heights
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, xWidth * zWidth * 4 * sizeof(float), NULL, GL_DYNAMIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, xWidth * zWidth * sizeof(float), heights);
    glBufferSubData(GL_ARRAY_BUFFER, xWidth * zWidth * sizeof(float), xWidth * zWidth * 3 * sizeof(float), normals);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    needSendGPU = false;
}

void HeightMesh::render() {
	glBindVertexArray(VAO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void HeightMesh::clean() {
    if (createdOnGPU) {
        glDeleteBuffers(1, &EBO);
        glDeleteBuffers(1, &VBO);
        glDeleteVertexArrays(1, &VAO);
    }
    
    if (generated) {
        delete[] heights;
        delete[] normals;
        delete[] indices;
    }

    createdOnGPU = false;
    generated = false;
    needSendGPU = false;
    numIndices = 0;
    VAO = 0;
    VBO = 0;
    EBO = 0;
}