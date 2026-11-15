#include "graphics/Camera.hpp"
#include <glm/geometric.hpp>
#include <cmath>
#include <stdexcept>
int main() {
    Camera a, b;
    a.orbit(80.0f, 0.0f);
    for (int i=0;i<100;++i) b.orbit(0.8f,0.0f);
    if (glm::distance(a.position(),b.position()) > 0.001f)
        throw std::runtime_error("Orbit depends on update count");
    a.zoom(100000.0f);
    a.orbit(0,100000.0f);
    const auto v=a.viewMatrix();
    for(int c=0;c<4;++c) for(int r=0;r<4;++r)
        if(!std::isfinite(v[c][r])) throw std::runtime_error("Invalid clamped view");
    a.reset(); Camera initial;
    if(glm::distance(a.position(),initial.position())>0.001f)
        throw std::runtime_error("Reset failed");
    glm::vec3 priorPosition(0);
    for(int preset=0;preset<4;++preset) {
        a.setPreset(preset);
        const auto matrix=a.viewMatrix();
        for(int c=0;c<4;++c) for(int r=0;r<4;++r)
            if(!std::isfinite(matrix[c][r])) throw std::runtime_error("Invalid camera preset");
        if(preset>0 && glm::distance(a.position(),priorPosition)<.1f)
            throw std::runtime_error("Presets must have distinct viewpoints");
        priorPosition=a.position();
    }
    a.setPreset(-100); b.setPreset(0);
    if(glm::distance(a.position(),b.position())>.001f)
        throw std::runtime_error("Negative preset was not clamped");
    a.setPreset(100); b.setPreset(3);
    if(glm::distance(a.position(),b.position())>.001f)
        throw std::runtime_error("Large preset was not clamped");
    a.setPreset(0);
    if(!a.orthographic()) throw std::runtime_error("Room preset must be parallel");
    const auto project=[&a](glm::vec3 p) {
        const auto clip=a.projectionMatrix(16.f/9.f)*a.viewMatrix()*glm::vec4(p,1);
        return glm::vec2(clip)/clip.w;
    };
    const auto nearEdge=project({2,1,2})-project({2,0,2});
    const auto farEdge=project({-3,1,-2})-project({-3,0,-2});
    if(glm::distance(nearEdge,farEdge)>.00001f)
        throw std::runtime_error("Parallel wall edges change scale with depth");
    const float originalScale=glm::length(nearEdge);
    a.zoom(1);
    if(glm::length(project({2,1,2})-project({2,0,2}))<=originalScale)
        throw std::runtime_error("Orthographic zoom failed");
    a.setOrthographic(false);
    if(std::abs(a.projectionMatrix(16.f/9.f)[2][3]+1.f)>.00001f)
        throw std::runtime_error("Perspective fallback failed");
}
