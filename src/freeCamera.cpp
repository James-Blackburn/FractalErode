#include "freeCamera.hpp"

FreeCamera::FreeCamera() {
    type = CameraTypes::FREE;
}

FreeCamera::FreeCamera(Vec3 position_, Vec3 up_, Vec3 front_, 
    float yaw_, float pitch_, float speed_, float sensitivity_) 
    : Camera(position_, up_, front_) {
    yaw = yaw_;
    pitch = pitch_;
    speed = speed_;
    sensitivity = sensitivity_;
    type = CameraTypes::FREE;
}

void FreeCamera::update(float deltaTime) {
    Window* window = Window::getInstance();
    static float mxOld = (float)window->width / 2.0f;
    static float myOld = (float)window->height / 2.0f;
    float mx = window->mx;
    float my = window->my;

    // update camera from cursor input
    // update the yaw and pitch of camera by how much the cursor has moved
    yaw += (mx - mxOld) * sensitivity * deltaTime;
    pitch += (myOld - my) * sensitivity * deltaTime;

    // clamp pitch
    if (pitch > 89.0f)
        pitch = 89.0f;
    else if (pitch < -89.0f)
        pitch = -89.0f;

    mxOld = mx;
    myOld = my;

    // update camera from keyboard input
    // move the position of the camera according to the 
    // direction of the front vector and keyboard inputs
    if (window->keys[GLFW_KEY_W]) {
        position = position + (front * speed * deltaTime);
    } if (window->keys[GLFW_KEY_S]) {
        position = position - (front * speed * deltaTime);
    } if (window->keys[GLFW_KEY_D]) {
        position = position + (normalize(cross(front, up)) * speed * deltaTime);
    } if (window->keys[GLFW_KEY_A]) {
        position = position - (normalize(cross(front, up)) * speed * deltaTime);
    }

    // move camera position up and down
    if (window->keys[GLFW_KEY_SPACE]) {
        position.y += speed * deltaTime;
    } if (window->keys[GLFW_KEY_LEFT_SHIFT]) {
        position.y -= speed * deltaTime;
    }
}

void FreeCamera::calculateEye() {
    front.x = cos(yaw * M_PI / 180.0f) * cos(pitch * M_PI / 180.0f);
    front.y = sin(pitch * M_PI / 180.0f);
    front.z = sin(yaw * M_PI / 180.0f) * cos(pitch * M_PI / 180.0f);
    front = normalize(front);
}