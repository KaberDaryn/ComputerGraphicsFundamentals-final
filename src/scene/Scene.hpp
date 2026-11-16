#pragma once
#include "scene/SceneNode.hpp"
#include "scene/WowSequence.hpp"
#include <glm/glm.hpp>
#include <algorithm>
#include <memory>
#include <array>
#include <cstdint>

class Mesh;
class Texture;
class Shader;

class Scene final {
public:
    Scene();
    ~Scene();
    Scene(const Scene&) = delete;
    Scene& operator=(const Scene&) = delete;
    void draw(const Shader& shader, RenderPass pass) const;
    // Monitor layouts: 0 = parallel, 1 = inward-angled, 2 = wide.
    void setMonitorLayout(int layout);
    int monitorLayout() const { return monitorLayout_; }
    void cycleMonitorLayout();
    // RGB modes: 0 = rainbow, 1 = solid, 2 = off.
    void setRgbMode(int mode);
    int rgbMode() const { return wow_.active() ? 0 : rgbMode_; }
    void cycleRgbMode();
    void setRgbSpeed(float speed);
    float rgbSpeed() const { return wow_.active() ? std::max(1.45f, rgbSpeed_) : rgbSpeed_; }
    void setRgbColor(const glm::vec3& color);
    glm::vec3 rgbColor() const { return rgbColor_; }
    glm::vec3 rgbLightColor(float time) const;
    void toggleLamp();
    bool lampOn() const { return lampOn_; }
    float lampBrightness() const {
        return wow_.active() ? wow_.values().lampLevel : (lampOn_ ? 1.0f : 0.0f);
    }
    void setDayFactor(float factor);
    float dayFactor() const { return wow_.active() ? wow_.values().daylight : dayFactor_; }
    void toggleDayNight();
    void setFeedEnabled(bool enabled) { feedEnabled_ = enabled; }
    bool feedEnabled() const { return feedEnabled_; }
    void setReadingLight(bool on) { readingLight_=on; }
    bool readingLight() const { return readingLight_; }
    void setAccentLight(bool on) { accentLight_=on; }
    bool accentLight() const { return accentLight_; }
    void setDrawersOpen(bool open) { drawersOpen_=open; }
    bool drawersOpen() const { return drawersOpen_; }
    void setScreenMode(int mode);
    int screenMode() const { return screenMode_; }
    void setShadows(bool enabled) { shadows_=enabled; }
    bool shadows() const { return shadows_; }
    std::uint64_t shadowRevision() const { return shadowRevision_; }
    void setGlow(bool enabled) { glow_=enabled; }
    bool glow() const { return glow_; }
    glm::vec3 lampPosition() const;
    void update(float deltaTime);
    void toggleFinale();
    bool finaleActive() const { return wow_.active(); }
    float finaleProgress() const { return wow_.progress(); }
    bool finaleRestoring() const {
        return wow_.phase() == WowSequence::Phase::Restoring;
    }
    const char* finalePhaseName() const { return wow_.phaseName(); }
    float roomLightMultiplier() const {
        return wow_.active() ? wow_.values().roomMultiplier : 1.0f;
    }
    float rgbBoost() const {
        return wow_.active() ? wow_.values().rgbBoost : 1.0f;
    }
private:
    // Resources outlive the hierarchy that references them.
    std::unique_ptr<Mesh> cube_, plane_, cylinder_, sphere_, screen_, torus_;
    std::unique_ptr<Texture> wood_, floor_, rug_, flag_, fabric_, mountains_, desktop_, terminal_;
    SceneNode root_{"Scene root"};
    SceneNode* bulb_ = nullptr;
    SceneNode* leftMonitor_ = nullptr;
    SceneNode* rightMonitor_ = nullptr;
    int monitorLayout_ = 0;
    int rgbMode_ = 0;
    float rgbSpeed_ = 1.0f;
    glm::vec3 rgbColor_{0.12f, 0.65f, 1.0f};
    bool lampOn_ = true;
    bool feedEnabled_ = true;
    float dayFactor_ = 1.0f;
    float targetDayFactor_ = 1.0f;
    WowSequence wow_{};
    bool readingLight_=true, accentLight_=true, drawersOpen_=false;
    bool shadows_=true, glow_=true;
    float drawerAmount_=0;
    int screenMode_=0;
    std::array<SceneNode*,3> drawers_{};
    SceneNode* leftScreen_=nullptr;
    SceneNode* readingBulb_=nullptr;
    SceneNode* readingShade_=nullptr;
    SceneNode* shelfStrip_=nullptr;
    std::uint64_t shadowRevision_=0;
};
