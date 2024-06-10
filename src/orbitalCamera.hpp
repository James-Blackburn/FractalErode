#ifndef ORBITAL_CAMERA_HPP_INCLUDED
#define ORBITAL_CAMERA_HPP_INCLUDED

#define _USE_MATH_DEFINES
#include <cmath>

#include "camera.hpp"
#include "window.hpp"

class OrbitalCamera : public Camera {
private:
    Vec3 sphericalCoords = { 0.0f, 0.0f, 0.0f };
public:
    float radius = 16.0f;
    float azimuth = 0.0f;
    float polar = 0.0f;
    float minRadius = 2.0f;
    float maxRadius = 400.0f;
    float zoomSpeed = 2.5f;
    float sensitivity = 0.001f;

    // constants
    static constexpr float MAX_AZIMUTH = static_cast<float>(M_PI) * 2.0f;
    static constexpr float MAX_POLAR = static_cast<float>(M_PI) / 2.0f - 0.01f;

    OrbitalCamera();
    OrbitalCamera(Vec3 position_, Vec3 up_, Vec3 front_, float radius_, float azimuth_, 
        float polar_, float minRadius_, float maxRadius_, float scollSpeed_, float sensitvity_);

    void calculateEye();
    void update(float deltaTime);
    Mat4 getViewProjection();
};

#endif