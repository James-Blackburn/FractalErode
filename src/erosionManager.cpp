#include "erosionManager.hpp"
#include "terrain.hpp"

#include <glad/glad.h>
#include <omp.h>
#include <future>
#include <algorithm>

#define heightIn terrain->heightmap
#define waterIn terrain->water

void ErosionManager::init(Terrain* terrain_) {
    terrain = terrain_;
    width = terrain->width;
    size = width * width;

    // compile compute shaders
    erosionShader = new ShaderProgram(std::vector<Shader>{
        {"res/shaders/erosion.comp", GL_COMPUTE_SHADER}});
    bufferUpdateShader = new ShaderProgram(std::vector<Shader>{
        {"res/shaders/erosionUpdate.comp", GL_COMPUTE_SHADER}});
    updateDeltaHShader = new ShaderProgram(std::vector<Shader>{
        {"res/shaders/erosionDeltaH.comp", GL_COMPUTE_SHADER}});

    // create CPU erosion buffers
    heightOut = std::vector<float>(size);
    sedimentIn = std::vector<float>(size);
    heightOut = std::vector<float>(size);
    sedimentOut = std::vector<float>(size);
    waterOut = std::vector<float>(size);

    // create GPU erosion buffers
    glGenBuffers(1, &heightInSSBO);   glGenBuffers(1, &heightOutSSBO);
    glGenBuffers(1, &waterInSSBO);     glGenBuffers(1, &waterOutSSBO);
    glGenBuffers(1, &sedimentInSSBO);  glGenBuffers(1, &sedimentOutSSBO);
    glGenBuffers(1, &totalDeltaHSSBO); glGenBuffers(1, &totalDeltaHWSSBO);

    // fill buffers
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, heightOutSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, size * sizeof(float), NULL, GL_DYNAMIC_COPY);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, waterOutSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, size * sizeof(float), NULL, GL_DYNAMIC_COPY);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, sedimentInSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, size * sizeof(float), NULL, GL_DYNAMIC_COPY);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, sedimentOutSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, size * sizeof(float), NULL, GL_DYNAMIC_COPY);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, totalDeltaHSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, size * sizeof(float), NULL, GL_DYNAMIC_COPY);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, totalDeltaHWSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, size * sizeof(float), NULL, GL_DYNAMIC_COPY);

    // bind SSBOs
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, heightInSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, heightOutSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, waterInSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, waterOutSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, sedimentInSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, sedimentOutSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, totalDeltaHWSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, totalDeltaHSSBO);
}

float ErosionManager::calculateScore() {
    std::vector<float> slopeMap(size);
    long double totalHeight = 0.0;
    long double totalVariance = 0.0;
    float mean, std;
    
    // calculate slope map
    #pragma omp parallel for
    for (unsigned int z = 2; z < width - 2; z++) {
        for (unsigned int x = 2; x < width - 2; x++) {
            // get maximum height difference between neighbours, Von Neumann neighbourhood
            float cellHeight = heightIn[z * width + x];
            slopeMap[z * width + x] = std::max({
                std::fabs(cellHeight - heightIn[(z + 1) * width + x]),
                std::fabs(cellHeight - heightIn[(z - 1) * width + x]),
                std::fabs(cellHeight - heightIn[z * width + x + 1]),
                std::fabs(cellHeight - heightIn[z * width + x - 1])
                });
            totalHeight += slopeMap[z * width + x];
        }
    }
    mean = totalHeight / (long double)(size);
    
    // caluclate total variance
    for (unsigned int z = 2; z < width - 2; z++) {
        for (unsigned int x = 2; x < width - 2; x++) {
            totalVariance += std::pow(slopeMap[z * width + x] - mean, 2);
        }
    }
    std = std::sqrt(totalVariance / (long double)(size));
    
    return std / mean;
}

void ErosionManager::startErosion(ErosionBackend backend) {
    step = 0;
    eroding = true;
    if (backend == ErosionBackend::CPU) {
        erosionFutureCPU = std::async(std::launch::async, &ErosionManager::erosionPipelineCPU, this);
    }
    else if (backend == ErosionBackend::GPU) {
        erosionPipelineGPU();
    }
}

void ErosionManager::stopErosion() {
    eroding = false;
    if (erosionFutureCPU.valid()) {
        erosionFutureCPU.wait();
    }
}

void ErosionManager::clean() {
    glDeleteBuffers(1, &heightInSSBO);
    glDeleteBuffers(1, &heightOutSSBO);
    glDeleteBuffers(1, &waterInSSBO);
    glDeleteBuffers(1, &waterOutSSBO);
    glDeleteBuffers(1, &sedimentInSSBO);
    glDeleteBuffers(1, &sedimentOutSSBO);
    glDeleteBuffers(1, &totalDeltaHWSSBO);
    glDeleteBuffers(1, &totalDeltaHSSBO);

    bufferUpdateShader->clean();
    updateDeltaHShader->clean();
    bufferUpdateShader->clean();
}

// CPU EROSION --------------------------------------------------------------------
void ErosionManager::erosionPipelineCPU() {
    // fill data grids
    #pragma omp parallel for
    for (int z = 1; z < width - 1; z++) {
        for (int x = 1; x < width - 1; x++) {
            const int cellIndex = z * width + x;
            waterIn[cellIndex] = rain * (heightIn[cellIndex] / terrain->maxHeight);
            sedimentIn[cellIndex] = 0.0f;

            waterOut[cellIndex] = waterIn[cellIndex];
            sedimentOut[cellIndex] = 0.0f;
            heightOut[cellIndex] = heightIn[cellIndex];
        }
    }

    while (eroding && step < nSteps) {
        // perform hydraulic erosion
        if (hydraulicEnabled) {
            // distribute water if it is time to rain
            if (rainFrequency) {
                if (step % rainFrequency == 0) {
                    distributeRainCPU();
                }
            }
            hydraulicErosionCPU();
        }
        // Thermal Weathering
        if (thermalEnabled)
            thermalErosionCPU();

        // use output array as input for next step
        #pragma omp parallel for
        for (int z = 1; z < width - 1; z++) {
            for (int x = 1; x < width - 1; x++) {
                const int cellIndex = z * width + x;

                // apply evaporation if any
                waterOut[cellIndex] *= kE;
                if (waterOut[cellIndex] < 0.000001f) {
                    heightOut[cellIndex] += sedimentIn[cellIndex];
                    sedimentOut[cellIndex] = 0.0f;
                    waterOut[cellIndex] = 0.0f;
                }

                heightIn[cellIndex] = heightOut[cellIndex];
                waterIn[cellIndex] = waterOut[cellIndex];
                sedimentIn[cellIndex] = sedimentOut[cellIndex];
            }
        }

        #pragma omp atomic
        step++;

        // generate mesh if needed
        if (terrain->showErosion && !terrain->needMeshSentGPU()) {
            terrain->generateMesh(terrain->showWater);
        }
    }

    // wait for any previous mesh upload to complete
    if (eroding) {
        while (terrain->needMeshSentGPU()) {}
        // generate terrain + water mesh
        terrain->generateMesh(true);
    }

    eroding = false;
}

void ErosionManager::distributeRainCPU() {
    #pragma omp parallel for
    for (int z = 1; z < width - 1; z++) {
        for (int x = 1; x < width - 1; x++) {
            const int cellIndex = (z * width) + x;
            waterIn[cellIndex] += rain * (heightIn[cellIndex] / terrain->maxHeight);
            waterOut[cellIndex] = waterIn[cellIndex];
        }
    }
}

void ErosionManager::hydraulicErosionCPU() {
    int neighbours[8];
    float neighboursDeltaH[8];
    
    static constexpr int dX[8] = { -1, +0, +1, -1, +1, -1, +0, +1 };
    static constexpr int dZ[8] = { -1, -1, -1, +0, +0, +1, +1, +1 };

    #pragma omp parallel for schedule(dynamic) private(neighbours, neighboursDeltaH)
    for (int z = 1; z < width - 1; z++) {
        for (int x = 1; x < width - 1; x++) {
            const int cellIndex = (z * width) + x;

            // skip if no water in current cell
            if (waterIn[cellIndex] == 0.0f)
                continue;

            // grab neighbours
            float totalDeltaH = 0.0f;

            for (int i = 0; i < 8; i++) {
                const int nCellIndex = ((z + dZ[i]) * width) + (x + dX[i]);
                // get total difference in height (inc. water)
                const float deltaH = (heightIn[cellIndex] + waterIn[cellIndex]) -
                    (heightIn[nCellIndex] + waterIn[nCellIndex]);

                if (deltaH > 0.0f) {
                    totalDeltaH += deltaH;
                }

                neighboursDeltaH[i] = deltaH;
                neighbours[i] = nCellIndex;
            }

            float cellTotalDeltaH = 0.0f;
            float cellTotalDeltaS = 0.0f;
            float cellTotalDeltaW = 0.0f;

            // for each neighbour calculate flow of water and sediment
            for (int n = 0; n < 8; n++) {
                const int nCellIndex = neighbours[n];
                const float deltaH = neighboursDeltaH[n];

                // try to move all the excess water out of the cell
                float deltaW = std::min(waterIn[cellIndex], deltaH);

                // neighbour total height (inc water) is higher than current cell
                if (deltaW <= 0.0f) {
                    // deposit some sediment at current cell if altitude is lower
                    if (heightIn[cellIndex] <= heightIn[nCellIndex]) {
                        const float sedDeposit = kD * sedimentIn[cellIndex];
                        cellTotalDeltaH += sedDeposit;
                        cellTotalDeltaS -= sedDeposit;
                    }
                }

                // neighbour total height (inc. water) is lower than current cell
                else {
                    // calculate movement of water from current cell to neighbour
                    // scale water to move by difference in heights
                    deltaW = deltaW * (deltaH / totalDeltaH);
                    #pragma omp atomic
                    waterOut[nCellIndex] += deltaW;
                    cellTotalDeltaW -= deltaW;

                    // sediment trying to move from cell to neighbour
                    const float deltaS = sedimentIn[cellIndex] * (deltaH / totalDeltaH);
                    // calculate max amount of sediment able to be carried in water at current cell
                    const float sCap = deltaW * kC;
                    if (deltaS >= sCap) { // deposition
                        // move max amount of sediment in to neighbouring cell
                        #pragma omp atomic
                        sedimentOut[nCellIndex] += sCap;
                        // deposit left over sediment in current cell
                        const float sedimentToDeposit = kD * (deltaS - sCap);
                        cellTotalDeltaS -= sedimentToDeposit + sCap;
                        cellTotalDeltaH += sedimentToDeposit;
                    }
                    else { // erosion
                        const float erosionAmount = kS * (sCap - deltaS);
                        cellTotalDeltaH -= erosionAmount;
                        cellTotalDeltaS -= deltaS;
                        #pragma omp atomic
                        sedimentOut[nCellIndex] += deltaS + erosionAmount;
                    }
                }
            }
            #pragma omp atomic
            heightOut[cellIndex] += cellTotalDeltaH;
            #pragma omp atomic
            sedimentOut[cellIndex] += cellTotalDeltaS;
            #pragma omp atomic
            waterOut[cellIndex] += cellTotalDeltaW;
        }
    }

}

void ErosionManager::thermalErosionCPU() {
    static constexpr int dX[8] = { -1, +0, +1, -1, +1, -1, +0, +1 };
    static constexpr int dZ[8] = { -1, -1, -1, +0, +0, +1, +1, +1 };
    
    float neighboursDeltaH[8];
    int neighbours[8];

    #pragma omp parallel for schedule(dynamic) private(neighbours, neighboursDeltaH)
    for (int z = 1; z < width - 1; z++) {
        for (int x = 1; x < width - 1; x++) {
            float cellTotalDeltaH = 0.0f;
            const int cellIndex = z * width + x;

            // get neighbours
            float totalDeltaH = 0.0f;
            int totalLowerNeighbours = 0;
            for (int i = 0; i < 8; i++) {
                // check if neighbour is not a boundary
                if (z + dZ[i] > 0 && z + dZ[i] < width - 1 &&
                    x + dX[i] > 0 && x + dX[i] < width - 1) {

                    const int nCellIndex = ((z + dZ[i]) * width) + (x + dX[i]);

                    // get difference in height 
                    const float deltaH = heightIn[cellIndex] - heightIn[nCellIndex];
                    if (deltaH > kT) {
                        totalDeltaH += deltaH;
                        neighboursDeltaH[totalLowerNeighbours] = deltaH;
                        neighbours[totalLowerNeighbours] = nCellIndex;
                        totalLowerNeighbours++;
                    }
                }
            }

            for (int i = 0; i < totalLowerNeighbours; i++) {
                const float deltaH = cT * (neighboursDeltaH[i] - kT) * (neighboursDeltaH[i] / totalDeltaH);
                cellTotalDeltaH -= deltaH;
                #pragma omp atomic
                heightOut[neighbours[i]] += deltaH;
            }
            #pragma omp atomic
            heightOut[cellIndex] += cellTotalDeltaH;
        }
    }
}
// END CPU EROSION ----------------------------------------------------------------

// GPU EROSION --------------------------------------------------------------------
void ErosionManager::erosionPipelineGPU() {
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, heightInSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, size * sizeof(float), heightIn.data(), GL_DYNAMIC_COPY);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, waterInSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, size * sizeof(float), waterIn.data(), GL_DYNAMIC_COPY);

    // send uniforms
    // Main erosion shader
    glUseProgram(erosionShader->glID);

    glUniform1i(0, width);
    glUniform1i(1, hydraulicEnabled);
    glUniform1f(2, kC);
    glUniform1f(3, kD);
    glUniform1f(4, kS);
    glUniform1f(5, kE);
    glUniform1i(6, thermalEnabled);
    glUniform1f(7, kT);
    glUniform1f(8, cT);

    // buffer update shader
    glUseProgram(bufferUpdateShader->glID);
    glUniform1i(0, width);
    glUniform1f(1, terrain->maxHeight);
    glUniform1f(3, rain);
    glUniform1i(4, rainFrequency);
    glUniform1f(5, kE);

    // deltaH shader, for neighbour heights
    glUseProgram(updateDeltaHShader->glID);
    glUniform1i(0, width);
    glUniform1i(1, kT);

    // dispatch the compute shader and await results
    for (step = 0; step < nSteps; step++) {
        // update buffers
        glUseProgram(bufferUpdateShader->glID);
        glUniform1i(2, step);
        glDispatchCompute(width / WORKGROUP_SIZE, width / WORKGROUP_SIZE, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

        // calculate new deltaH values
        glUseProgram(updateDeltaHShader->glID);
        glDispatchCompute(width / WORKGROUP_SIZE, width / WORKGROUP_SIZE, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

        // run simulation on buffers
        glUseProgram(erosionShader->glID);
        glDispatchCompute(width / WORKGROUP_SIZE, width / WORKGROUP_SIZE, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    }

    // fetch data from GPU
    // only required data is the heightmap and water values
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, heightInSSBO);
    glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, size * sizeof(float), heightIn.data());
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, waterInSSBO);
    glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, size * sizeof(float), waterIn.data());
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    glUseProgram(0);
}
// END GPU EROSION ----------------------------------------------------------------
