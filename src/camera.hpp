#ifndef CAMERA_HPP_INCLUDED
#define CAMERA_HPP_INCLUDED

#define _USE_MATH_DEFINES

#include "vec3.hpp"
#include "mat4.hpp"

enum class CameraTypes {
    BASE,
    ORBITAL,
    FREE
};

class Camera {
protected:
    CameraTypes type = CameraTypes::BASE;
public:
    Vec3 position{ 0.0f, 0.0f, 0.0f };
    Vec3 up{ 0.0f, 1.0f, 0.0f };
    Vec3 front{ 0.0f, 0.0f, 1.0f };

    Camera() = default;
    Camera(Vec3 position_, Vec3 up_, Vec3 front_);

    virtual void calculateEye() = 0;
    virtual void update(float deltaTime) = 0;
    virtual Mat4 getViewProjection();
    inline CameraTypes getType();
};

CameraTypes Camera::getType() {
    return type;
}

#endif