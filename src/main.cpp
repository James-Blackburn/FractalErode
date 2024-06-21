#define _USE_MATH_DEFINES

#include "window.hpp"
#include "shaderProgram.hpp"
#include "texture.hpp"
#include "terrain.hpp"
#include "skybox.hpp"
#include "vec3.hpp"
#include "mat4.hpp"
#include "orbitalCamera.hpp"
#include "freeCamera.hpp"
#include "heightMesh.hpp"
#include "tree.hpp"
#include "noise.hpp"
#include "camera.hpp"

#include <iostream>
#include <vector>

// Request high performance GPU
#ifdef _WIN32
extern "C" {
    _declspec(dllexport) unsigned long NvOptimusEnablement = 1;
    _declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}
#endif

namespace {
    Window* window;

    // environment
    Terrain terrainPatch;
    Skybox skybox;

    // cameras
    OrbitalCamera orbitalCamera;
    FreeCamera freeCamera;
    Camera* camera;
    Mat4 projection;

    // shaders
    ShaderProgram* terrainShader;
    ShaderProgram* waterShader;
    ShaderProgram* cubemapShader;
    ShaderProgram* treeShader;

    // textures
    Texture* treeTexture;
    Texture* grassTexture;
    Texture* grassTextureNormal;
    Texture* stoneTexture;
    Texture* stoneTextureNormal;
    Texture* snowTexture;
    Texture* snowTextureNormal;
    Texture* mudTextureNormal;
    Texture* mudTexture;
    
    // lighting
    Vec3 lightDirection{ -1.0f, 1.0f, 1.0f };
    Vec3 lightColour{ 1.0f, 1.0f, 0.95f };
    Vec3 bgColour{ 0.644f, 0.797f, 1.0f };
    Vec3 fogColour{ 0.644f, 0.744f, 0.844f };
    Vec3 fogDistances{ 200.0f, 700.f };
   
    // misc
    float score = 0.0f;
    bool wireframe = false;
    float fps = 0.0f;
    float deltaTime = 0.0f;
    bool showErosion = true;
    bool showWater = true;
    bool showTrees = true;
    const char* terrainSizes[5] = { "256", "512", "1024", "2048", "4096" };
    int selectedTerrainSize = 2;
    int cameraTypeToggle = 0;

    void defineUI();
    void handleEvents();
    void loadShaders();
    void loadTextures();
    void renderScene();
};

int main()
{
    window = Window::getInstance();
    
    // create window
    if (window->init(1920, 1080, "FractalErode", true) == -1){
        std::cout << "Error creating window" << std::endl;
        return -1;
    }

    // load textures and shaders
    loadTextures();
    loadShaders();

    // set up projections
    projection = perspective(45.0f, 
        (float)Window::getInstance()->width / (float)Window::getInstance()->height, 
        0.05f, 1100.0f
    );

    // generate terrain heightmap and send to GPU
    terrainPatch.generateHeightmap(atoi(terrainSizes[selectedTerrainSize]));
    terrainPatch.generateMesh();
    terrainPatch.sendMeshGPU();

    // initialise cameras
    float terrainCenter = terrainPatch.width * terrainPatch.scale / 2.0f;
    orbitalCamera = OrbitalCamera({terrainCenter, terrainPatch.maxHeight / 2.5f, terrainCenter},
        { 0.0f, 1.0f, 0.0f }, {0.0f, 0.0f, 0.0f}, 200.0f, 0.82f, 0.35f, 2.0f, 400.0f, 75.0f, 0.05f);
    freeCamera = FreeCamera({terrainCenter, terrainPatch.maxHeight, terrainCenter},
        {0.0f, 1.0f, 0.0f}, {1.0f, -0.3f, 1.0f}, 45.0f, 0.0f, 20.0f, 1.0f);
    camera = &orbitalCamera;

    // render loop
    float oTime = static_cast<float>(glfwGetTime());
    while (!window->windowShouldClose())
    {
        // calculate time between frames in seconds
        float time = static_cast<float>(glfwGetTime());
        deltaTime = time - oTime;
        fps = 1.0f / deltaTime;
        oTime = time;

        // update meshes if needed
        if (terrainPatch.needMeshSentGPU())
            terrainPatch.sendMeshGPU();

        // event handling
        glfwPollEvents();
        handleEvents();

        // define imGUI UI elements and handle logic
        if (camera->getType() == CameraTypes::ORBITAL)
            defineUI();

        // clear window
        glClearColor(bgColour.x, bgColour.y, bgColour.z, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // render
        renderScene();
        if (camera->getType() == CameraTypes::ORBITAL) {
            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        }

        // swap buffers, updating window contents
        window->update();
    }

    // stop erosion
    if (terrainPatch.getErosionStatus())
        terrainPatch.erosionManager.stopErosion();

    // clean resources
    terrainShader->clean();
    waterShader->clean();
    cubemapShader->clean();
    treeShader->clean();

    treeTexture->clean();
    grassTexture->clean();
    grassTextureNormal->clean();
    stoneTexture->clean();
    stoneTextureNormal->clean();
    snowTexture->clean();
    snowTextureNormal->clean();
    mudTextureNormal->clean();
    mudTexture->clean();

    terrainPatch.clean();
    window->clean();

    return 0;
}

namespace {
    
    void loadShaders() {
        std::cout << "Loading and compiling shaders..." << std::endl;
        // compile shaders
        terrainShader = new ShaderProgram(std::vector<Shader>{
            {"res/shaders/terrain.vert", GL_VERTEX_SHADER},
            {"res/shaders/terrain.frag", GL_FRAGMENT_SHADER}});
        waterShader = new ShaderProgram(std::vector<Shader>{
            {"res/shaders/water.vert", GL_VERTEX_SHADER},
            {"res/shaders/water.frag", GL_FRAGMENT_SHADER}});
        cubemapShader = new ShaderProgram(std::vector<Shader>{
            {"res/shaders/cubemap.vert", GL_VERTEX_SHADER},
            {"res/shaders/cubemap.frag", GL_FRAGMENT_SHADER}});
        treeShader = new ShaderProgram(std::vector<Shader>{
            {"res/shaders/billboard.vert", GL_VERTEX_SHADER},
            {"res/shaders/billboard.frag", GL_FRAGMENT_SHADER}});

        // set texture binding points
        glUseProgram(terrainShader->glID);
        glUniform1i(9, 0);  
        glUniform1i(10, 1);
        glUniform1i(11, 2); 
        glUniform1i(12, 3);
        glUniform1i(13, 4);
        glUniform1i(14, 5);
        glUniform1i(15, 6); 
        glUniform1i(16, 7);

        // bind textures to shaders
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, stoneTexture->glID);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, stoneTextureNormal->glID);

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, grassTexture->glID);
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, grassTextureNormal->glID);

        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, mudTexture->glID);
        glActiveTexture(GL_TEXTURE5);
        glBindTexture(GL_TEXTURE_2D, mudTextureNormal->glID);

        glActiveTexture(GL_TEXTURE6);
        glBindTexture(GL_TEXTURE_2D, snowTexture->glID);
        glActiveTexture(GL_TEXTURE7);
        glBindTexture(GL_TEXTURE_2D, snowTextureNormal->glID);
    }

    void loadTextures() {
        std::cout << "Loading textures..." << std::endl;
        // load textures
        treeTexture = new Texture("res/textures/tree1.png");
        grassTexture = new Texture("res/textures/forrest_ground_01_2k.gltf/textures/forrest_ground_01_diff_2k.jpg");
        stoneTexture = new Texture("res/textures/rock_08_2k.gltf/textures/rock_08_diff_2k.jpg");
        snowTexture = new Texture("res/textures/macro_flour_2k.gltf/textures/macro_flour_diff_2k.jpg");
        mudTexture = new Texture("res/textures/brown_mud_2k.gltf/textures/brown_mud_diff_2k.jpg");
        grassTextureNormal = new Texture("res/textures/forrest_ground_01_2k.gltf/textures/forrest_ground_01_nor_gl_2k.jpg");
        stoneTextureNormal = new Texture("res/textures/rock_08_2k.gltf/textures/rock_08_nor_gl_2k.jpg");
        snowTextureNormal = new Texture("res/textures/macro_flour_2k.gltf/textures/macro_flour_nor_gl_2k.jpg");
        mudTextureNormal = new Texture("res/textures/brown_mud_2k.gltf/textures/brown_mud_nor_gl_2k.jpg");

        // skybox
        std::vector<const char*> skyboxFaces{
            "res/skybox/bluecloud_rt.jpg",
            "res/skybox/bluecloud_lf.jpg",
            "res/skybox/bluecloud_up.jpg",
            "res/skybox/bluecloud_dn.jpg",
            "res/skybox/bluecloud_bk.jpg",
            "res/skybox/bluecloud_ft.jpg"
        };
        skybox.load(skyboxFaces);
    }

    void renderScene() {
        // get uniform data
        Mat4 cameraViewProjection = camera->getViewProjection();
        Mat4 model = make_scaling(terrainPatch.scale, 1.0f, terrainPatch.scale);
        Mat4 mvp = projection * cameraViewProjection * model;
        Vec3 lightDirectionNorm = normalize(lightDirection);

        // draw window contents
        // draw terrain
        glUseProgram(terrainShader->glID);
        glUniformMatrix4fv(0, 1, GL_TRUE, &mvp.m00);
        glUniformMatrix4fv(1, 1, GL_TRUE, &model.m00);
        glUniform1i(2, terrainPatch.width);
        glUniform3fv(3, 1, &camera->position.x);
        glUniform3fv(4, 1, &lightDirectionNorm.x);
        glUniform3fv(5, 1, &lightColour.x);
        glUniform2fv(6, 1, &fogDistances.x);
        glUniform3fv(7, 1, &fogColour.x);
        glUniform1i(8, showWater);
        glUniform1f(17, terrainPatch.maxHeight);
        terrainPatch.renderTerrain();

        // render water
        if (showWater && terrainPatch.erosionManager.hydraulicEnabled) {
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            glUseProgram(waterShader->glID);
            glUniformMatrix4fv(0, 1, GL_TRUE, &mvp.m00);
            glUniformMatrix4fv(1, 1, GL_TRUE, &model.m00);
            glUniform1i(2, terrainPatch.width);
            glUniform3fv(3, 1, &camera->position.x);
            glUniform3fv(4, 1, &lightDirectionNorm.x);
            glUniform3fv(5, 1, &lightColour.x);
            glUniform2fv(6, 1, &fogDistances.x);
            glUniform3fv(7, 1, &fogColour.x);
            glUniform1f(8, terrainPatch.maxHeight);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_CUBE_MAP, skybox.getTexture());
            terrainPatch.renderWater();

            glDisable(GL_BLEND);
        }

        // render skybox
        glUseProgram(cubemapShader->glID);
        glUniformMatrix4fv(0, 1, GL_TRUE, &projection.m00);
        Mat4 view = cameraViewProjection;
        view.m03 = 0.0f;
        view.m13 = 0.0f;
        view.m23 = 0.0f;
        glUniformMatrix4fv(1, 1, GL_TRUE, &view.m00);
        glUniform3fv(2, 1, &fogColour.x);
        skybox.render();

        // render trees
        if (showTrees) {
            glUseProgram(treeShader->glID);
            glUniformMatrix4fv(0, 1, GL_TRUE, &projection.m00);
            glUniformMatrix4fv(1, 1, GL_TRUE, &cameraViewProjection.m00);
            glUniformMatrix4fv(3, 1, GL_TRUE, &model.m00);
            glUniform3fv(4, 1, &camera->position.x);
            glUniform3fv(5, 1, &lightDirectionNorm.x);
            glUniform3fv(6, 1, &lightColour.x);
            glUniform2fv(7, 1, &fogDistances.x);
            glUniform3fv(8, 1, &fogColour.x);
            glUniform1f(9, terrainPatch.maxHeight);
            glUniform1i(2, 8);
            glActiveTexture(GL_TEXTURE8);
            glBindTexture(GL_TEXTURE_2D, treeTexture->glID);
            terrainPatch.renderTrees();
        }
    }

    void handleEvents() {
        // toggle camera mode
        if ((window->keys[GLFW_KEY_ENTER] && !cameraTypeToggle)) {
            if (camera->getType() == CameraTypes::ORBITAL) {
                window->hideCursor();
                camera = &freeCamera;
            } else if (camera->getType() == CameraTypes::FREE) {
                window->showCursor();
                camera = &orbitalCamera;
            }
            cameraTypeToggle = 1;
        }
        else if (!window->keys[GLFW_KEY_ENTER] && cameraTypeToggle) {
            cameraTypeToggle = 0;
        }

        // camera controls for terrain
        // only if UI is not being interacted with
        if (!ImGui::GetIO().WantCaptureMouse || camera->getType() == CameraTypes::FREE)
            camera->update(deltaTime);

        // update where camera is looking
        camera->calculateEye();
    }

    void defineUI() {
        // Start the ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        if (ImGui::BeginMainMenuBar()) {
            // Terrain Menu
            if (ImGui::BeginMenu("Terrain")) {
                // options for generating heightmap
                ImGui::Text("Heightmap Parameters");
                ImGui::Combo("size", &selectedTerrainSize, terrainSizes, IM_ARRAYSIZE(terrainSizes));
                ImGui::SliderInt("octaves", &terrainPatch.nOctaves, 1, 16);
                ImGui::SliderFloat("frequency", &terrainPatch.frequency, 0.001f, 0.01f);
                ImGui::SliderFloat("amplitude", &terrainPatch.amplitude, 1.0f, 400.0f);
                ImGui::SliderFloat("persistence", &terrainPatch.persistence, 0.0f, 0.75f);
                ImGui::SliderFloat("lacunarity", &terrainPatch.lacunarity, 1.0f, 4.0f);
                ImGui::SliderFloat("domain warp", &terrainPatch.domainWarpAmplitude, 0.0f, 1000.0f);
                ImGui::SliderInt("seed", &terrainPatch.seed, 0, 100);

                // if the generate button has been clicked
                if (ImGui::Button("Generate")) {
                    // regenerate terrain based on updated values
                    terrainPatch.erosionManager.stopErosion();
                    terrainPatch.clean();
                    terrainPatch.generateHeightmap(atoi(terrainSizes[selectedTerrainSize]));
                    terrainPatch.generateMesh(false);
                    terrainPatch.sendMeshGPU();

                    // recenter cameras
                    orbitalCamera.position.x = terrainPatch.width * terrainPatch.scale / 2.0f;
                    orbitalCamera.position.y = terrainPatch.maxHeight / 2.5f;
                    orbitalCamera.position.z = terrainPatch.width * terrainPatch.scale / 2.0f;
                    freeCamera.position.x = orbitalCamera.position.x;
                    freeCamera.position.y = terrainPatch.maxHeight;
                    freeCamera.position.z = orbitalCamera.position.z;
                }

                ImGui::EndMenu();
            }

            // visualisation menu
            if (ImGui::BeginMenu("Visualisation")) {
                // toggle wireframe
                if (ImGui::Checkbox("wireframe mode", &wireframe)) {
                    if (wireframe)
                        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                    else
                        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                }
                if (!terrainPatch.getErosionStatus()) {
                    ImGui::Checkbox("show erosion", &showErosion);
                    if (showErosion)
                        ImGui::Checkbox("show water", &showWater);
                    else
                        showWater = false;
                    ImGui::Checkbox("show trees", &showTrees);

                    // only update if meshes are not pending a send
                    if (!terrainPatch.needMeshSentGPU()) {
                        terrainPatch.showErosion = showErosion;
                        terrainPatch.showWater = showWater;
                        terrainPatch.showTrees = showTrees;
                    }
                    if (ImGui::Button("Update Terrain Altitude"))
                        terrainPatch.updateAltitude();
                }

                ImGui::EndMenu();
            }

            // Erosion Menu
            if (ImGui::BeginMenu("Erosion")) {
                // options for eroding terrain
                ImGui::Text("Hydraulic Erosion Parameters");
                ImGui::SliderFloat("sediment capacity", &terrainPatch.erosionManager.kC, 0.0f, 1.0f);
                ImGui::SliderFloat("deposition rate", &terrainPatch.erosionManager.kD, 0.0f, 1.0f);
                ImGui::SliderFloat("dissolving rate", &terrainPatch.erosionManager.kS, 0.0f, 1.0f);
                ImGui::SliderFloat("evaporation rate", &terrainPatch.erosionManager.kE, 0.0f, 1.0f);
                ImGui::SliderFloat("rain amount", &terrainPatch.erosionManager.rain, 0.0f, 0.5f);
                ImGui::SliderInt("rain frequency", &terrainPatch.erosionManager.rainFrequency, 0, 500);
                ImGui::Text("Thermal Weathering Parameters");
                ImGui::SliderFloat("talus angle", &terrainPatch.erosionManager.kT, 0.0f, 1.0f);
                ImGui::SliderFloat("weathering rate", &terrainPatch.erosionManager.cT, 0.0f, 0.1f);
                ImGui::Text("General");
                ImGui::Checkbox("enable hydraulic", &terrainPatch.erosionManager.hydraulicEnabled);
                ImGui::Checkbox("enable thermal", &terrainPatch.erosionManager.thermalEnabled);
                ImGui::SliderInt("iterations", &terrainPatch.erosionManager.nSteps, 1, 5000);
                
                if (ImGui::Button("Erode CPU"))
                    terrainPatch.erosionManager.startErosion(ErosionBackend::CPU);
                if (ImGui::Button("Erode GPU")) {
                    terrainPatch.erosionManager.startErosion(ErosionBackend::GPU);
                    terrainPatch.generateMesh(true);
                }
                
                if (terrainPatch.getErosionStatus()) {
                    ImGui::Text("Erosion Step: %d", terrainPatch.erosionManager.step);
                }
                else {
                    ImGui::Text("Not currently eroding");
                    if (ImGui::Button("Calculate Erosion Score")) {
                        score = terrainPatch.erosionManager.calculateScore();
                    }
                    ImGui::Text("Score: %f", score);
                }

                ImGui::EndMenu();
            }

            // Lighting Menu
            if (ImGui::BeginMenu("Lighting")) {
                ImGui::SliderFloat3("lighting direction", &lightDirection.x, -1.0f, 1.0f);
                ImGui::SliderFloat3("lighting colour", &lightColour.x, 0.0f, 1.0f);
                ImGui::SliderFloat3("backdrop colour", &bgColour.x, 0.0f, 1.0f);

                ImGui::EndMenu();
            }

            // Camera Menu
            if (ImGui::BeginMenu("Camera")) {
                ImGui::Text("-- Press [ENTER] to switch camera modes --");
               
                if (camera->getType() == CameraTypes::ORBITAL)
                    ImGui::Text("Current camera type: ORBITAL");
                else if (camera->getType() == CameraTypes::FREE)
                    ImGui::Text("Current camera type: FREE");

                ImGui::Text("Orbital Camera Controls");
                ImGui::SliderFloat("azimuth angle", &orbitalCamera.azimuth, 0.0f, OrbitalCamera::MAX_AZIMUTH);
                ImGui::SliderFloat("polar angle", &orbitalCamera.polar, -OrbitalCamera::MAX_POLAR, OrbitalCamera::MAX_POLAR);
                ImGui::SliderFloat("radius", &orbitalCamera.radius, orbitalCamera.minRadius, orbitalCamera.maxRadius);

                ImGui::EndMenu();
            }

            ImGui::EndMainMenuBar();

            // Perform debug calculations
            const unsigned int hmapMem = (terrainPatch.size * sizeof(float)) / 1048576u;

            ImGui::Begin("Debug");
            ImGui::Text("FPS: %.1f", fps);
            ImGui::Text("FPSAVG60: %.1f", ImGui::GetIO().Framerate);
            ImGui::Text("HMAP_SIZE: %dx%d", terrainPatch.width, terrainPatch.width);
            ImGui::Text("CELL_SCALE: %f", terrainPatch.scale);
            ImGui::Text("HMAP_MEM: %dMB", hmapMem);
            ImGui::End();
        }
    }
}
