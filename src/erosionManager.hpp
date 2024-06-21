#ifndef EROSION_MANAGER_HPP_INCLUDED
#define EROSION_MANAGER_HPP_INCLUDED

#include "shaderProgram.hpp"

#include <memory>
#include <atomic>
#include <vector>
#include <cmath>
#include <future>

class Terrain;

enum class ErosionBackend {
    CPU, GPU
};

class ErosionManager {
private:
    constexpr static unsigned int WORKGROUP_SIZE = 32;
    std::future<void> erosionFutureCPU;
    Terrain* terrain = nullptr;

    ShaderProgram* bufferUpdateShader;
    ShaderProgram* updateDeltaHShader;
    ShaderProgram* erosionShader;

    // CPU erosion buffers
    std::vector<float> heightOut;
    std::vector<float> waterOut;
    std::vector<float> sedimentIn;
    std::vector<float> sedimentOut;

    // GPU erosion buffers
    unsigned int heightInSSBO = NULL;
    unsigned int heightOutSSBO = NULL;
    unsigned int waterInSSBO = NULL;
    unsigned int waterOutSSBO = NULL;
    unsigned int sedimentInSSBO = NULL;
    unsigned int sedimentOutSSBO = NULL;
    unsigned int totalDeltaHWSSBO = NULL;
    unsigned int totalDeltaHSSBO = NULL;

    // CPU erosion functions
    void erosionPipelineCPU();
    void distributeRainCPU();
    void hydraulicErosionCPU();
    void thermalErosionCPU();

    // GPU erosion functions
    void erosionPipelineGPU();
public:
    // Erosion parameters
    int nSteps = 2500; // NUMBER OF ITERATIONS
    // Hydraulic Erosion
    bool hydraulicEnabled = true;
    float kC = 0.75f; // SEDIMENT CAPACITY
    float kD = 0.015f; // DEPOSITION RATE
    float kS = 0.15f; // DISSOLVING RATE
    float kE = 1.0f; // EVAPORATION RATE // 0.995f // 0.999f
    float rain = 0.150f; // 0.01f // 0.01f
    int rainFrequency = 0; // 50.0f // 100
    // Thermal Weathering
    bool thermalEnabled = true;
    float kT = 0.6f; // GLOBAL TALUS ANGLE
    float cT = 0.05f; // THERMAL WEATHERING RATE

    bool eroding = false;
    int step = 0;

    unsigned int width = 0;
    unsigned int size = 0;
    
    ErosionManager() = default;

	void init(Terrain* terrain_);
    void clean();
    float calculateScore();
    
    void startErosion(ErosionBackend backend);
    void stopErosion();
};

#endif