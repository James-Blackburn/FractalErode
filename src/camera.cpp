#include "camera.hpp"

Camera::Camera(Vec3 position_, Vec3 up_, Vec3 front_) {
    position = position_;
    front = front_;
    up = up_;
}

Mat4 Camera::getViewProjection() {
    // calculate vector at right angle to the front and up vector of camrea
    Vec3 xAxis = normalize(cross(front, up));
    // calculate vector coming out the top of the camera
    Vec3 yAxis = normalize(cross(xAxis, front));
    Vec3 zAxis = { -front.x, -front.y, -front.z };
    return {
        xAxis.x, xAxis.y, xAxis.z, -dot(xAxis, position),
        yAxis.x, yAxis.y, yAxis.z, -dot(yAxis, position),
        zAxis.x, zAxis.y, zAxis.z, -dot(zAxis, position),
        0.0f, 0.0f, 0.0f, 1.0f
    };
}