#ifndef FREE_CAMERA_HPP_INCLUDED
#define FREE_CAMERA_HPP_INCLUDED

#define _USE_MATH_DEFINES
#include <cmath>

#include "camera.hpp"
#include "window.hpp"

class FreeCamera : public Camera {
public:
    float yaw = 0.0f;
    float pitch = 0.0f;
    float speed = 1.0f;
    float sensitivity = 0.1f;

    FreeCamera();
    FreeCamera(Vec3 position_, Vec3 up_, Vec3 front_, float yaw_, 
        float pitch_, float speed_, float sensitivity_);

    void calculateEye();
    void update(float deltaTime);
};

#endif