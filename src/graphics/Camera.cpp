#include "graphics/Camera.hpp"
#include "scene/RoomLayout.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <cmath>

Camera::Camera() { reset(); }
void Camera::reset() {
    orthographic_=false;
    yaw_ = glm::radians(42.0f);
    pitch_ = glm::radians(22.0f);
    distance_ = 7.5f;
    target_ = {0.0f, 1.0f, 0.0f};
}
void Camera::orbit(float deltaX, float deltaY) {
    yaw_ += glm::radians(deltaX);
    pitch_ = std::clamp(pitch_ + glm::radians(deltaY),
                        glm::radians(-3.0f), glm::radians(85.0f));
}
void Camera::setPreset(int preset) {
    struct Pose { glm::vec3 target; float yaw, pitch, distance; };
    const Pose poses[] = {
        {{RoomLayout::centerX,RoomLayout::wallHeight*.5f,RoomLayout::centerZ},45,35.2643897f,14.8f},
        {{0,1.3f,0},35,23,7.2f},
        {{-3.f,1.1f,1.5f},70,20,6.f},
        {{RoomLayout::centerX,.7f,RoomLayout::centerZ},0,75,13.f}};
    const auto& p=poses[std::clamp(preset,0,3)];
    target_=p.target; yaw_=glm::radians(p.yaw); pitch_=glm::radians(p.pitch); distance_=p.distance;
    orthographic_=true;
}
void Camera::zoom(float wheelOffset) {
    distance_ = std::clamp(distance_ - wheelOffset * 0.5f, 2.0f, 24.0f);
}
glm::vec3 Camera::position() const {
    const float c = std::cos(pitch_);
    return target_ + distance_ * glm::vec3(
        c * std::sin(yaw_), std::sin(pitch_), c * std::cos(yaw_));
}
glm::mat4 Camera::viewMatrix() const {
    return glm::lookAt(position(), target_, glm::vec3(0.0f, 1.0f, 0.0f));
}
glm::mat4 Camera::projectionMatrix(float aspect) const {
    if(orthographic_) {
        // Preserve zoom while fitting a tall/narrow viewport without clipping
        // the room horizontally. Parallel walls keep a consistent scale.
        const float halfHeight=distance_*std::tan(glm::radians(fov_*.5f))*std::max(1.f,1.20f/aspect);
        return glm::ortho(-halfHeight*aspect,halfHeight*aspect,-halfHeight,halfHeight,.1f,100.f);
    }
    return glm::perspective(glm::radians(fov_), aspect, 0.1f, 100.0f);
}
