#ifndef TERRAIN_HPP_INCLUDED
#define TERRAIN_HPP_INCLUDED

#include "vec3.hpp"
#include "heightMesh.hpp"
#include "waterMesh.hpp"
#include "terrainMesh.hpp"
#include "shaderProgram.hpp"
#include "tree.hpp"
#include "erosionManager.hpp"

#include <vector>
#include <thread>

class Terrain {
private:
    static constexpr int TREE_MIN_DISTANCE = 4;
    static constexpr int TREE_CHANCE = 10;
    static constexpr float TERRAIN_BIAS = 0.025f;

    TerrainMesh terrainMesh;
    WaterMesh waterMesh;
    InstancedTree trees;
public:
    // heightmap parameters
    unsigned int width = 0;
    unsigned int size = 0;
    float scale = 0.25f;
    int nOctaves = 12;
    float frequency = 0.005f;
    float amplitude = 300.0f;
    float persistence = 0.5f;
    float lacunarity = 2.0f;
    int seed = 0;
    float domainWarpAmplitude = 400.0f;
    float maxHeight = 0.0f;
    float minHeight = 30.0f;

    ErosionManager erosionManager;
    
    std::vector<float> heightmap;
    std::vector<float> water;
    std::vector<float> altitude;
    std::vector<Vec3> treePositions;
    std::vector<int> treeIndexes;

    // threadsafe flags
    std::atomic<bool> showErosion = true;
    std::atomic<bool> showWater = true;
    std::atomic<bool> showTrees = true;
    std::atomic<bool> stopEroding = false;
    std::atomic<bool> treesUpdated = false;

    Terrain() = default;
    Terrain(unsigned int, float);
    void generateHeightmap(int);
    void generateMesh(bool genWater=false);
    void generateWaterMesh();
    void sendMeshGPU();
    void updateAltitude();
    void clean();

    inline bool needMeshSentGPU() const;
    inline float getLastHmapGenTime() const;
    inline float getLastErosionTime() const;
    inline bool getErosionStatus() const;
    inline void setErosionStatus(bool status_);
    inline void renderTerrain();
    inline void renderWater();
    inline void renderTrees();
};

// if meshes are pending a GPU send
bool Terrain::needMeshSentGPU() const {
    return terrainMesh.needSendGPU || waterMesh.needSendGPU || treesUpdated;
}

bool Terrain::getErosionStatus() const {
    return erosionManager.eroding;
}

void Terrain::setErosionStatus(bool status) {
    erosionManager.eroding = status;
}

void Terrain::renderTerrain() {
    terrainMesh.render();
}

void Terrain::renderWater() {
    waterMesh.render();
}

void Terrain::renderTrees() {
    trees.render();
}

//float Terrain::getLastHmapGenTime() const {
 //   return timeGenHmap;
//}

//float Terrain::getLastErosionTime() const {
//    return erosionManager.timeErode;
//}

#endif