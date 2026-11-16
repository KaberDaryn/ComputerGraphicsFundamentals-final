#include "scene/Scene.hpp"
#include "scene/SceneFactory.hpp"
#include "graphics/Primitives.hpp"
#include "graphics/Texture.hpp"
#include <algorithm>
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>
#include <stdexcept>

Scene::Scene()
    : cube_(Primitives::cube()), plane_(Primitives::plane()),
      cylinder_(Primitives::cylinder()), sphere_(Primitives::sphere()),
      screen_(Primitives::screenQuad()),
      torus_(Primitives::torus()),
      wood_(std::make_unique<Texture>("assets/textures/wood.png")),
      floor_(std::make_unique<Texture>("assets/textures/oak_floor.png")),
      rug_(std::make_unique<Texture>("assets/textures/rug.png")),
      flag_(std::make_unique<Texture>("assets/textures/kazakhstan_flag.png")),
      fabric_(std::make_unique<Texture>("assets/textures/fabric.png")),
      mountains_(std::make_unique<Texture>("assets/textures/mountains.png")),
      desktop_(std::make_unique<Texture>("assets/textures/desktop.png")),
      terminal_(std::make_unique<Texture>("assets/textures/terminal.png")) {
    SceneAssets assets{cube_.get(), plane_.get(), cylinder_.get(), sphere_.get(),
                       screen_.get(), wood_.get(), floor_.get(), rug_.get(), flag_.get(),
                       torus_.get(),fabric_.get(),mountains_.get(),desktop_.get()};
    SceneFactory::buildRoom(root_, assets);
    SceneNode& desk = SceneFactory::buildDesk(root_, assets);
    leftMonitor_ = desk.findByName("Left monitor");
    rightMonitor_ = desk.findByName("Right monitor");
    if (!leftMonitor_ || !rightMonitor_)
        throw std::runtime_error("Monitor nodes missing from desk hierarchy");
    bulb_ = &SceneFactory::buildLamp(desk, assets);
    SceneFactory::buildChair(root_, assets);
    SceneFactory::buildStudio(root_, assets);
    for(int i=0;i<3;++i) drawers_[i]=root_.findByName("Office drawer "+std::to_string(i));
    leftScreen_=root_.findByName("Left screen content");
    readingBulb_=root_.findByName("Reading lamp glow");
    readingShade_=root_.findByName("Linen lamp shade");
    shelfStrip_=root_.findByName("Shelf warm light");
    if(!leftScreen_ || !readingBulb_ || !readingShade_ || !shelfStrip_ ||
       std::any_of(drawers_.begin(),drawers_.end(),[](auto* p){return !p;}))
        throw std::runtime_error("Studio control nodes missing");
    leftScreen_->unlit=true;
}
Scene::~Scene() = default;
void Scene::setScreenMode(int mode) {
    screenMode_=std::clamp(mode,0,2);
    Texture* textures[]={desktop_.get(),terminal_.get(),mountains_.get()};
    leftScreen_->texture=textures[screenMode_];
}
void Scene::draw(const Shader& shader, RenderPass pass) const {
    root_.draw(shader, glm::mat4(1.0f), pass);
}
void Scene::toggleLamp() {
    if (wow_.active()) return;
    lampOn_ = !lampOn_;
    bulb_->emission = glm::vec3(0.90f, 0.65f, 0.30f) * lampBrightness();
}
glm::vec3 Scene::lampPosition() const { return bulb_->worldPosition(); }

void Scene::setMonitorLayout(int layout) {
    ++shadowRevision_;
    monitorLayout_ = std::clamp(layout, 0, 2);
    // Keep each monitor and its stand as a single rigid transform group.
    struct Pose { float x, z, yaw; };
    const Pose left[3] = {{-0.86f, -0.47f, 0.0f},
                          {-0.86f, -0.47f, 18.0f},
                          {-1.05f, -0.47f, 4.0f}};
    const Pose right[3] = {{0.86f, -0.47f, 0.0f},
                           {0.86f, -0.47f, -18.0f},
                           {1.05f, -0.47f, -4.0f}};
    auto transform = [](SceneNode& node, const Pose& pose) {
        node.localTransform = glm::translate(glm::mat4(1.0f),
                                              {pose.x, 0.0f, pose.z}) *
            glm::rotate(glm::mat4(1.0f), glm::radians(pose.yaw),
                        glm::vec3(0.0f, 1.0f, 0.0f));
    };
    transform(*leftMonitor_, left[monitorLayout_]);
    transform(*rightMonitor_, right[monitorLayout_]);
}
void Scene::cycleMonitorLayout() { setMonitorLayout((monitorLayout_ + 1) % 3); }
void Scene::setRgbMode(int mode) {
    if (!wow_.active()) rgbMode_ = std::clamp(mode, 0, 2);
}
void Scene::cycleRgbMode() { setRgbMode((rgbMode_ + 1) % 3); }
void Scene::setRgbSpeed(float speed) {
    if (!wow_.active()) rgbSpeed_ = std::clamp(speed, 0.1f, 5.0f);
}
void Scene::setRgbColor(const glm::vec3& color) {
    if (wow_.active()) return;
    rgbColor_ = glm::clamp(color, glm::vec3(0.0f), glm::vec3(1.0f));
}
glm::vec3 Scene::rgbLightColor(float time) const {
    if (rgbMode() == 2) return glm::vec3(0.0f);
    if (rgbMode() == 1) return rgbColor_;
    // Same three-channel cosine palette and phase speed as rgb.frag.
    constexpr float tau = 6.28318530718f;
    const float phase = time * rgbSpeed() * 0.16f;
    return glm::vec3(0.5f + 0.5f * std::cos(tau * phase),
                     0.5f + 0.5f * std::cos(tau * (phase + 1.0f/3.0f)),
                     0.5f + 0.5f * std::cos(tau * (phase + 2.0f/3.0f)));
}

void Scene::setDayFactor(float factor) {
    if (wow_.active()) return;
    dayFactor_ = targetDayFactor_ = std::clamp(factor, 0.0f, 1.0f);
}
void Scene::toggleDayNight() {
    if (!wow_.active()) targetDayFactor_ = targetDayFactor_ >= 0.5f ? 0.0f : 1.0f;
}

void Scene::toggleFinale() {
    if (!wow_.active()) {
        // The controller saves values, while original UI settings remain intact.
        targetDayFactor_ = dayFactor_;
        wow_.start(dayFactor_, lampOn_ ? 1.0f : 0.0f);
    } else {
        wow_.restore(); // Also works during the initial fade.
    }
}
void Scene::update(float deltaTime) {
    const float previousDrawerAmount=drawerAmount_;
    drawerAmount_+=std::clamp((drawersOpen_?1.f:0.f)-drawerAmount_,
        -std::max(0.f,deltaTime)*2.f,std::max(0.f,deltaTime)*2.f);
    if(previousDrawerAmount!=drawerAmount_) ++shadowRevision_;
    for(auto* drawer:drawers_) drawer->localTransform[3].z=.17f+.33f*drawerAmount_;
    if (!wow_.active()) {
        const float step = std::max(0.0f, deltaTime) * 0.55f;
        dayFactor_ += std::clamp(targetDayFactor_ - dayFactor_, -step, step);
    }
    wow_.update(deltaTime);
    const float room=roomLightMultiplier();
    readingBulb_->emission=readingLight_?glm::vec3(1.2f,.65f,.22f)*room:glm::vec3(0);
    readingShade_->emission=readingLight_?glm::vec3(.20f,.11f,.045f)*room:glm::vec3(0);
    shelfStrip_->emission=accentLight_?glm::vec3(.90f,.45f,.14f)*room:glm::vec3(0);
    // Only a material uniform changes; static vertex buffers never re-upload.
    bulb_->emission = glm::vec3(0.90f, 0.65f, 0.30f) * lampBrightness();
}
