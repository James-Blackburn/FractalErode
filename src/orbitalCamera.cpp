#include "orbitalCamera.hpp"

OrbitalCamera::OrbitalCamera() {
    type = CameraTypes::ORBITAL;
}

OrbitalCamera::OrbitalCamera(Vec3 position_, Vec3 up_, Vec3 front_, float radius_, float azimuth_,
    float polar_, float minRadius_, float maxRadius_, float zoomSpeed_, float sensitvity_) :
    Camera(position_, up_, front_) {
    radius = radius_;
    azimuth = azimuth_;
    polar = polar_;
    minRadius = minRadius_;
    maxRadius = maxRadius_;
    zoomSpeed = zoomSpeed_;
    sensitivity = sensitvity_;
    type = CameraTypes::ORBITAL;
}

void OrbitalCamera::update(float deltaTime) {
    Window* window = Window::getInstance();
    static float mxOld = (float)window->width / 2.0f;
    static float myOld = (float)window->height / 2.0f;
    static bool mouseDown = false;
    float mx = window->mx;
    float my = window->my;

    // only pan camera if mouse button down
    if (window->mouseButtons[GLFW_MOUSE_BUTTON_1] == GLFW_PRESS) {
        // move camera horizontal
        azimuth += (mx - mxOld) * sensitivity * deltaTime;
        if (azimuth > MAX_AZIMUTH)
            azimuth = 0.0f;
        else if (azimuth < 0.0f)
            azimuth = MAX_AZIMUTH;

        // move camera vertical
        polar += (my - myOld) * sensitivity * deltaTime;
        if (polar > MAX_POLAR)
            polar = MAX_POLAR;
        else if (polar < -MAX_POLAR)
            polar = -MAX_POLAR;
    }

    mxOld = mx;
    myOld = my;

    // zoom camera
    radius -= window->scroll * zoomSpeed * deltaTime;
    if (radius < minRadius)
        radius = minRadius;
    else if (radius > maxRadius)
        radius = maxRadius;

    window->scroll = 0.0f;
}

void OrbitalCamera::calculateEye(){
    sphericalCoords.x = position.x + radius * cos(polar) * cos(azimuth);
    sphericalCoords.y = position.y + radius * sin(polar);
    sphericalCoords.z = position.z + radius * cos(polar) * sin(azimuth);
    front = normalize(position - sphericalCoords);
}

Mat4 OrbitalCamera::getViewProjection() {
    // calculate vector at right angle to the front and up vector of camera
    Vec3 xAxis = normalize(cross(front, up));
    // calculate vector coming out the top of the camera
    Vec3 yAxis = normalize(cross(xAxis, front));
    Vec3 zAxis = { -front.x, -front.y, -front.z };
    return {
        xAxis.x, xAxis.y, xAxis.z, -dot(xAxis, sphericalCoords),
        yAxis.x, yAxis.y, yAxis.z, -dot(yAxis, sphericalCoords),
        zAxis.x, zAxis.y, zAxis.z, -dot(zAxis, sphericalCoords),
        0.0f, 0.0f, 0.0f, 1.0f
    };
}