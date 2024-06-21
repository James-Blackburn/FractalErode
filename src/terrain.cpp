#include "terrain.hpp" 
#include "vec3.hpp"
#include "noise.hpp"
#include "window.hpp"

#include <cstdlib>
#include <iostream>
#include <algorithm>
#include <chrono>
#include <omp.h>

Terrain::Terrain(unsigned int aHeightmapSize, float aScale) {
    width = aHeightmapSize;
    size = width * width;
    scale = aScale;
    generateHeightmap(aHeightmapSize);
    generateMesh(false);
}

void Terrain::generateHeightmap(int width_) {
    width = width_;
    size = width * width;

    std::atomic<float> heightMaxAtomic = 0.0f;
    heightmap = std::vector<float>(size);
    water = std::vector<float>(size);
    altitude = std::vector<float>(size);

    #pragma omp parallel for
    for (int z = 1; z < width - 1; z++){
        for (int x = 1; x < width - 1; x++){
            float dx = 0.0f; 
            float dz = 0.0f;

            if (domainWarpAmplitude > 0.0f) {
                dx = domainWarpAmplitude * perlinOctave(6.0f, 0.001f, 0.5f, 2.0f, (float)x - 1.4f, (float)z - 4.7f);
                dz = domainWarpAmplitude * perlinOctave(6.0f, 0.001f, 0.5f, 2.0f, (float)x + 5.2f, (float)z + 1.3f);
            }

            // get noise values at current x,z coordinate
            const float baseNoise = perlinOctave(4.0f, frequency, 0.5f, 2.0f,
                (x * scale) + ((float)seed) * width, (z * scale) + ((float)seed) * width);
            const float mountainNoise = perlinOctave(nOctaves, frequency, persistence, lacunarity,
                ((x + dx) * scale) + ((float)seed + 1.0f) * width, ((z + dz) * scale) + ((float)seed + 1.0f) * width);
            
            // use noise value to get a height
            const float height = minHeight + baseNoise * std::pow(mountainNoise, 2.0f) * amplitude;
            heightmap[(z * width) + x] = height;
            altitude[(z * width) + x] = height;
            
            if (height > heightMaxAtomic) {
                heightMaxAtomic = height;
            }
        }
    }
    maxHeight = heightMaxAtomic;

    // place trees
    trees.init(12.0f, 8.0f);
    for (int z = 1; z < width - 1; z += TREE_MIN_DISTANCE) {
        for (int x = 1; x < width - 1; x += TREE_MIN_DISTANCE) {
            const int cellIndex = z * width + x;
            if (heightmap[cellIndex] < 85.0f) {
                if (rand() % TREE_CHANCE == 0) {
                    treeIndexes.push_back(cellIndex);
                    treePositions.push_back(Vec3{ (float)x, heightmap[cellIndex] - 0.2f, (float)z });
                }
            }
        }
    }
    treesUpdated = true;

    // initialise meshes and erosion manager
    terrainMesh.init(1.0f, width, width);
    waterMesh.init(1.0f, width, width);
    erosionManager.init(this);
}

void Terrain::generateMesh(bool genWater){
    terrainMesh.generate(altitude.data(), heightmap.data());

    // generate water mesh
    if (genWater) {
        generateWaterMesh();
    }

    // update trees
    auto indexIterator = treeIndexes.begin();
    auto posIterator = treePositions.begin();
    const Vec3* terrainNormals = terrainMesh.getNormals();
    while (indexIterator != treeIndexes.end()) {
        const int index = *indexIterator;
        
        // calculate grass weight
        const float grad = abs(dot(terrainNormals[index], Vec3{ 0.0f, 1.0f, 0.0f }));
        float grassWeight = hermite(grad, 0.5f, 0.75f) * (1.0f - hermite(heightmap[index], 75.0f, 90.0f));
        grassWeight *= 1.0f - hermite(abs(altitude[index] - heightmap[index]), 0.0, 0.1f);

        if (grassWeight < 0.5f) {
            indexIterator = treeIndexes.erase(indexIterator);
            posIterator = treePositions.erase(posIterator);
            treesUpdated = true;
        }
        else {
            ++indexIterator;
            ++posIterator;
        }
    }
}

void Terrain::generateWaterMesh() {
    float* waterHeights = new float[(width * width)];
    float* smoothedHeights = new float[(width * width)];

#pragma omp parallel for
    for (int i = 0; i < (width*width); i++) {
        waterHeights[i] = heightmap[i] + water[i];
    }
    
    // smooth heights to improve water visuals
#pragma omp parallel for
    for (int z = 1; z < width - 1; z++) {
        for (int x = 1; x < width - 1; x++) {
            const float smoothedHeight = (
                 waterHeights[(z - 1) * width + x - 1] +
                 waterHeights[(z - 1) * width + x] +
                 waterHeights[(z - 1) * width + x + 1] +
                 waterHeights[z * width + x - 1] +
                 waterHeights[z * width + x] +
                 waterHeights[z * width + x + 1] +
                 waterHeights[(z + 1) * width + x - 1] +
                 waterHeights[(z + 1) * width + x] +
                 waterHeights[(z + 1) * width + x + 1]
             ) / 9.0f;
            smoothedHeights[z * width + x] = smoothedHeight;
        }
    }
    
    waterMesh.generate(terrainMesh.getVBO(), smoothedHeights);
    
    delete[] waterHeights;
    delete[] smoothedHeights;
}

void Terrain::sendMeshGPU() {
    if (terrainMesh.needSendGPU) {
        terrainMesh.sendGPU();
    }
    if (waterMesh.needSendGPU) {
        waterMesh.sendGPU();
    }
    if (treesUpdated) {
        trees.setPositions(treePositions);
        treesUpdated = false;
    }
}

void Terrain::updateAltitude() {
    for (int i = 0; i < width * width; i++) {
        altitude[i] = heightmap[i];
    }
    generateMesh();
    terrainMesh.updateAltitude();
}

void Terrain::clean(){
    terrainMesh.clean();
    waterMesh.clean();
    trees.clean();

    treeIndexes.clear();
    treePositions.clear();
    erosionManager.clean();
}
