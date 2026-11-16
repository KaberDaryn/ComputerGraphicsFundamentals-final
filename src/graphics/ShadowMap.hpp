#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>

// One filtered depth map shared by both cameras. Allocated once per GL context.
class ShadowMap final {
public:
    ShadowMap();
    ~ShadowMap();
    ShadowMap(const ShadowMap&) = delete;
    ShadowMap& operator=(const ShadowMap&) = delete;
    void begin() const;
    void bindTexture(unsigned int unit) const;
    static glm::mat4 lightMatrix();
private:
    GLuint framebuffer_ = 0, depth_ = 0;
    static constexpr int resolution = 2048;
};
