#include "app/Application.hpp"
#include "graphics/Shader.hpp"
#include "graphics/Framebuffer.hpp"
#include "graphics/ShadowMap.hpp"
#include "graphics/Primitives.hpp"
#include "scene/Lighting.hpp"
#include "scene/Scene.hpp"
#include "ui/UiPanel.hpp"
#include <glad/glad.h>
#include <algorithm>
#include <cstddef>
#include <glm/gtc/matrix_transform.hpp>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <cstdlib>
#include <fstream>
#include <vector>
#include <numeric>
#include <cstdint>
#include <limits>

namespace {
void errorCallback(int error, const char* message) {
    std::cerr << "GLFW error " << error << ": " << message << '\n';
}

// Draw all world geometry for either camera. The live-screen surface itself
// is deliberately excluded here: only the final onscreen pass samples the FBO.
void renderWorld(const Scene& scene, const Shader& lit, const Shader& rgb,
                 const Shader& sky, const glm::mat4& view,
                 const glm::mat4& projection, const glm::vec3& eye, float time) {
    lit.use();
    lit.setMat4("uView", view);
    lit.setMat4("uProjection", projection);
    lit.setVec3("uViewPosition", eye);
    Lighting::upload(lit, scene, time);
    lit.setBool("uShadowsEnabled",scene.shadows());
    scene.draw(lit, RenderPass::Lit);

    rgb.use();
    rgb.setMat4("uView", view);
    rgb.setMat4("uProjection", projection);
    rgb.setFloat("uTime", time);
    rgb.setFloat("uRgbSpeed", scene.rgbSpeed());
    rgb.setInt("uRgbMode", scene.rgbMode());
    rgb.setFloat("uRgbIntensity", scene.rgbBoost());
    rgb.setVec3("uRgbStaticColor", scene.rgbColor());
    scene.draw(rgb, RenderPass::RGB);

    sky.use();
    sky.setMat4("uView", view);
    sky.setMat4("uProjection", projection);
    sky.setFloat("uDayFactor", scene.dayFactor());
    sky.setFloat("uTime", time);
    scene.draw(sky, RenderPass::Sky);

    // The studio has one transparent side panel. Draw after opaque geometry,
    // without depth writes, and from both sides when the camera orbits.
    lit.use();
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);
    glDisable(GL_CULL_FACE);
    scene.draw(lit,RenderPass::Glass);
    glEnable(GL_CULL_FACE);
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
}

void clearScene(float daylight) {
    const glm::vec3 night(0.009f, 0.017f, 0.044f);
    const glm::vec3 day(0.22f, 0.35f, 0.54f);
    const glm::vec3 c = glm::mix(night, day, daylight);
    glClearColor(c.r, c.g, c.b, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

// Read the application's own back buffer, independently of desktop capture.
// PPM keeps this diagnostic path dependency-free; normal runs never read back.
void captureFrame(const std::string& mode, int width, int height) {
    const GLenum priorError=glGetError();
    if(priorError!=GL_NO_ERROR)
        throw std::runtime_error("OpenGL error before capture: "+std::to_string(priorError));
    std::vector<std::uint8_t> pixels(static_cast<size_t>(width) * height * 3);
    GLint alignment = 4;
    glGetIntegerv(GL_PACK_ALIGNMENT, &alignment);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadBuffer(GL_BACK);
    glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());
    const GLenum readError=glGetError();
    glPixelStorei(GL_PACK_ALIGNMENT, alignment);
    if(readError!=GL_NO_ERROR)
        throw std::runtime_error("OpenGL readback error: "+std::to_string(readError));
    std::ofstream file("capture_" + mode + ".ppm", std::ios::binary);
    file << "P6\n" << width << " " << height << "\n255\n";
    const auto stride = static_cast<size_t>(width) * 3;
    for (int row = height - 1; row >= 0; --row)
        file.write(reinterpret_cast<const char*>(pixels.data() + row * stride),
                   static_cast<std::streamsize>(stride));
    if (!file) throw std::runtime_error("Cannot save frame capture");
    std::cout << "Saved capture_" << mode << ".ppm (" << width << 'x' << height << ")\n";
}
}
Application::Application() {
    glfwSetErrorCallback(errorCallback);
    if (!glfwInit()) throw std::runtime_error("GLFW initialization failed");
    glfwReady_ = true;
    try {
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
#endif
        glfwWindowHint(GLFW_SAMPLES, 4);
        window_ = glfwCreateWindow(1280, 720,
                      "Dream Developer Desk", nullptr, nullptr);
        if (!window_) throw std::runtime_error("Cannot create OpenGL 3.3 context");
        glfwMakeContextCurrent(window_);
        if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
            throw std::runtime_error("GLAD initialization failed");
        if (!GLAD_GL_VERSION_3_3)
            throw std::runtime_error("OpenGL 3.3 Core required");
        glfwSwapInterval(1);
        glfwSetWindowUserPointer(window_, this);
        glfwSetCursorPosCallback(window_, cursorCallback);
        glfwSetScrollCallback(window_, scrollCallback);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glEnable(GL_MULTISAMPLE);
        std::cout << "OpenGL version: " << glGetString(GL_VERSION) << '\n';
    } catch (...) {
        if (window_) glfwDestroyWindow(window_);
        window_ = nullptr;
        glfwTerminate();
        glfwReady_ = false;
        throw;
    }
}
Application::~Application() {
    if (window_) glfwDestroyWindow(window_);
    if (glfwReady_) glfwTerminate();
}
void Application::cursorCallback(GLFWwindow* window, double x, double y) {
    auto* self = static_cast<Application*>(glfwGetWindowUserPointer(window));
    if (!self) return;
    bool down = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
    if (UiPanel::wantsMouse()) down = false;
    if (down && self->dragging_)
        self->camera_.orbit(float((x - self->lastX_) * 0.22),
                            float((self->lastY_ - y) * 0.22));
    self->dragging_ = down;
    self->lastX_ = x;
    self->lastY_ = y;
}
void Application::scrollCallback(GLFWwindow* window, double, double dy) {
    auto* self = static_cast<Application*>(glfwGetWindowUserPointer(window));
    if (self && !UiPanel::wantsMouse()) self->camera_.zoom(float(dy));
}
bool Application::pressedOnce(int key) {
    const bool down = glfwGetKey(window_, key) == GLFW_PRESS;
    const bool firstPress = down && !heldKeys_[static_cast<size_t>(key)];
    heldKeys_[static_cast<size_t>(key)] = down;
    return firstPress;
}
void Application::processInput(float dt, Scene& scene, UiPanel& ui) {
    if (glfwGetKey(window_, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window_, true);
    // Always sample key edges, even when a UI text field captures input.
    const bool toggleLamp = pressedOnce(GLFW_KEY_L);
    const bool changeMonitors = pressedOnce(GLFW_KEY_M);
    const bool changeRgb = pressedOnce(GLFW_KEY_R);
    const bool toggleHelp = pressedOnce(GLFW_KEY_H);
    const bool toggleDay = pressedOnce(GLFW_KEY_N);
    const bool toggleFeed = pressedOnce(GLFW_KEY_P);
    const bool toggleFinale = pressedOnce(GLFW_KEY_F);
    const bool drawers = pressedOnce(GLFW_KEY_B);
    const bool reading = pressedOnce(GLFW_KEY_T);
    const bool accent = pressedOnce(GLFW_KEY_Y);
    const bool content = pressedOnce(GLFW_KEY_C);
    const bool views[] = {pressedOnce(GLFW_KEY_1),pressedOnce(GLFW_KEY_2),
                          pressedOnce(GLFW_KEY_3),pressedOnce(GLFW_KEY_4)};
    if (UiPanel::wantsKeyboard()) return;

    float angle = 80.0f * dt;
    if (glfwGetKey(window_, GLFW_KEY_A) == GLFW_PRESS) camera_.orbit(-angle, 0);
    if (glfwGetKey(window_, GLFW_KEY_D) == GLFW_PRESS) camera_.orbit(angle, 0);
    if (glfwGetKey(window_, GLFW_KEY_W) == GLFW_PRESS) camera_.orbit(0, angle);
    if (glfwGetKey(window_, GLFW_KEY_S) == GLFW_PRESS) camera_.orbit(0, -angle);
    if (glfwGetKey(window_, GLFW_KEY_Q) == GLFW_PRESS) camera_.zoom(-3.0f * dt);
    if (glfwGetKey(window_, GLFW_KEY_E) == GLFW_PRESS) camera_.zoom(3.0f * dt);

    if (toggleFinale) scene.toggleFinale();
    if (toggleLamp) scene.toggleLamp();
    if (changeMonitors) scene.cycleMonitorLayout();
    if (changeRgb) scene.cycleRgbMode();
    if (toggleHelp) ui.toggleVisible();
    if (toggleDay) scene.toggleDayNight();
    if (toggleFeed) scene.setFeedEnabled(!scene.feedEnabled());
    if (drawers) scene.setDrawersOpen(!scene.drawersOpen());
    if (reading && !scene.finaleActive()) scene.setReadingLight(!scene.readingLight());
    if (accent && !scene.finaleActive()) scene.setAccentLight(!scene.accentLight());
    if (content) scene.setScreenMode((scene.screenMode()+1)%3);
    for(int i=0;i<4;++i) if(views[i]) camera_.setPreset(i);
    if (glfwGetKey(window_, GLFW_KEY_LEFT_BRACKET) == GLFW_PRESS)
        scene.setRgbSpeed(scene.rgbSpeed() - 0.8f * dt);
    if (glfwGetKey(window_, GLFW_KEY_RIGHT_BRACKET) == GLFW_PRESS)
        scene.setRgbSpeed(scene.rgbSpeed() + 0.8f * dt);
}
void Application::run() { runScene(); }
void Application::runScene() {
    // All GPU objects must be destroyed while this GL context is current.
    Shader lit("assets/shaders/basic.vert", "assets/shaders/basic.frag");
    Shader rgb("assets/shaders/rgb.vert", "assets/shaders/rgb.frag");
    Shader sky("assets/shaders/sky.vert", "assets/shaders/sky.frag");
    Shader screen("assets/shaders/screen.vert", "assets/shaders/screen.frag");
    Shader depth("assets/shaders/shadow.vert","assets/shaders/shadow.frag");
    Shader post("assets/shaders/post.vert","assets/shaders/post.frag");
    ShadowMap shadowMap;
    const auto lightMatrix=ShadowMap::lightMatrix();
    std::uint64_t renderedShadowRevision=std::numeric_limits<std::uint64_t>::max();
    auto fullscreen=Primitives::screenQuad();
    std::unique_ptr<Framebuffer> output;
    Scene scene;
    const char* benchmarkMode = std::getenv("DDD_BENCHMARK");
    const bool benchmark = benchmarkMode != nullptr;
    // Give wall decor and the larger rug breathing room in interactive views.
    // Benchmarks retain the original camera for comparable before/after runs.
    if (!benchmark) {
        camera_.setPreset(0);
    }
    if (benchmark) {
        glfwSwapInterval(0);
        int bw=0,bh=0; glfwGetFramebufferSize(window_, &bw, &bh);
        std::cout << "Framebuffer: " << bw << "x" << bh << "; VSync=0; warmup=2s; sample=15s\n";
        scene.setFeedEnabled(std::string(benchmarkMode) != "off");
        std::cout << "Renderer: " << glGetString(GL_RENDERER) << "\nVendor: "
                  << glGetString(GL_VENDOR) << "\n";
    }
    std::vector<double> frameTimes;
    bool finaleStarted = false;
    const double benchmarkStart = glfwGetTime();
    Framebuffer secondaryFeed(640, 368); // approximately the monitor's ratio
    UiPanel ui(window_);

    const char* captureSetting = std::getenv("DDD_CAPTURE");
    const std::string captureMode = captureSetting ? captureSetting : "";
    if (benchmark && captureSetting)
        throw std::runtime_error("Use DDD_CAPTURE and DDD_BENCHMARK separately");
    if (captureSetting) {
        const std::vector<std::string> modes{"day", "night", "finale", "restore",
            "side", "top", "angled", "wide", "feed-off", "lamp-off",
            "lounge","desk","drawers","terminal","wallpaper","lights-off","effects-off","resize","panel"};
        if (std::find(modes.begin(), modes.end(), captureMode) == modes.end())
            throw std::runtime_error("Unknown DDD_CAPTURE scenario");
        if(captureMode!="panel") ui.toggleVisible();
        if (captureMode == "night") scene.setDayFactor(0.0f);
        if (captureMode == "finale" || captureMode == "restore") scene.toggleFinale();
        if (captureMode == "side") camera_.orbit(-65.0f, 12.0f);
        if (captureMode == "top") camera_.orbit(-42.0f, 45.0f);
        if (captureMode == "angled") scene.setMonitorLayout(1);
        if (captureMode == "wide") scene.setMonitorLayout(2);
        if (captureMode == "feed-off") scene.setFeedEnabled(false);
        if (captureMode == "lamp-off") scene.toggleLamp();
        if (captureMode == "lounge") camera_.setPreset(2);
        if (captureMode == "desk") camera_.setPreset(1);
        if (captureMode == "top") camera_.setPreset(3);
        if (captureMode == "drawers") { camera_.setPreset(1); scene.setDrawersOpen(true); }
        if (captureMode == "terminal") { camera_.setPreset(1); scene.setScreenMode(1); }
        if (captureMode == "wallpaper") { camera_.setPreset(1); scene.setScreenMode(2); }
        if (captureMode == "lights-off") {
            scene.setDayFactor(0); scene.toggleLamp(); scene.setReadingLight(false); scene.setAccentLight(false);
        }
        if (captureMode == "effects-off") { scene.setShadows(false); scene.setGlow(false); }
    }
    const double captureStart = glfwGetTime();
    bool captureRestored = false;
    bool captureResized = false;

    lit.use();
    lit.setInt("uTexture", 0);
    lit.setInt("uShadowMap",1);
    lit.setMat4("uLightMatrix",lightMatrix);
    depth.use();
    depth.setMat4("uLightMatrix",lightMatrix);
    post.use();
    post.setInt("uImage",0);
    screen.use();
    screen.setInt("uCameraFeed", 0);

    // Second camera is intentionally different from the orbit camera.
    // Its fixed viewpoint captures the desk while the user moves freely.
    const glm::vec3 securityEye(-2.70f, 2.72f, 2.40f);
    const glm::mat4 securityView = glm::lookAt(
        securityEye, glm::vec3(0.0f, 1.38f, -0.38f), glm::vec3(0, 1, 0));
    const glm::mat4 securityProjection = glm::perspective(
        glm::radians(65.0f),
        float(secondaryFeed.width()) / float(secondaryFeed.height()),
        0.10f, 100.0f);

    double previous = glfwGetTime(), elapsed = 0.0;
    float fps = 0.0f;
    int frameCount = 0;
    while (!glfwWindowShouldClose(window_)) {
        const double now = glfwGetTime();
        const double rawDt = now - previous;
        previous = now;
        glfwPollEvents();
        if(captureMode=="resize" && !captureResized && now-captureStart>=1.0) {
            glfwSetWindowSize(window_,1440,900);
            captureResized=true;
        }
        const float dt = float(std::clamp(rawDt, 0.0, 0.05));
        if (!benchmark && !captureSetting) processInput(dt, scene, ui);
        if (captureMode == "restore" && !captureRestored && now - captureStart >= 5.0) {
            scene.toggleFinale();
            captureRestored = true;
        }
        if (benchmark && !finaleStarted && std::string(benchmarkMode) == "finale" && now-benchmarkStart >= 2.0) {
            scene.toggleFinale(); finaleStarted = true;
        }
        scene.update(dt);

        int width = 0, height = 0;
        glfwGetFramebufferSize(window_, &width, &height);
        if (width > 0 && height > 0) {
            if(!output || output->width()!=width || output->height()!=height) {
                output=std::make_unique<Framebuffer>(width,height);
                renderedShadowRevision=std::numeric_limits<std::uint64_t>::max();
            }
            if(scene.shadows() && scene.shadowRevision()!=renderedShadowRevision) {
                shadowMap.begin();
                depth.use();
                glEnable(GL_POLYGON_OFFSET_FILL);
                glPolygonOffset(2.f,4.f);
                scene.draw(depth,RenderPass::Shadow);
                glDisable(GL_POLYGON_OFFSET_FILL);
                Framebuffer::unbind();
                renderedShadowRevision=scene.shadowRevision();
            }
            shadowMap.bindTexture(1);
            // PASS A: render a second live camera view into a color texture.
            // Screen geometry is omitted to prevent sampling our current FBO.
            if (scene.feedEnabled()) {
                secondaryFeed.bind();
                glViewport(0, 0, secondaryFeed.width(), secondaryFeed.height());
                clearScene(scene.dayFactor());
                renderWorld(scene, lit, rgb, sky, securityView,
                            securityProjection, securityEye, float(now));
                Framebuffer::unbind();
            }

            // PASS B: main orbit camera to the default framebuffer.
            // Restore viewport; the offscreen framebuffer has a different size.
            output->bind();
            // Leave a dedicated strip for the control panel in wide windows.
            // With H hidden, the room recentres into the entire framebuffer.
            int logicalWidth=0;
            glfwGetWindowSize(window_,&logicalWidth,nullptr);
            const int panelWidth=!benchmark && ui.visible() && logicalWidth>=1000 ? int(390.f*width/logicalWidth) : 0;
            const int sceneWidth=width-panelWidth;
            glViewport(panelWidth, 0, sceneWidth, height);
            clearScene(scene.dayFactor());
            const glm::mat4 view = camera_.viewMatrix();
            const glm::mat4 projection = camera_.projectionMatrix(
                float(sceneWidth) / float(height));
            renderWorld(scene, lit, rgb, sky, view, projection,
                        camera_.position(), float(now));

            // PASS C: map the pre-rendered FBO texture to the right monitor.
            // Texture binding is not a CPU upload and does not rebuild meshes.
            if (scene.feedEnabled()) {
                screen.use();
                screen.setMat4("uView", view);
                screen.setMat4("uProjection", projection);
                secondaryFeed.bindColorTexture(0);
                scene.draw(screen, RenderPass::Screen);
                glBindTexture(GL_TEXTURE_2D, 0);
            }

            Framebuffer::unbind();
            glViewport(0,0,width,height);
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_CULL_FACE);
            post.use();
            post.setVec2("uPixel",{1.f/width,1.f/height});
            post.setBool("uGlow",scene.glow());
            output->bindColorTexture(0);
            fullscreen->draw();
            glEnable(GL_DEPTH_TEST);
            glEnable(GL_CULL_FACE);

            ui.beginFrame();
            ui.draw(scene, camera_, fps);
            ui.endFrame();
            const double captureDelay = captureMode == "restore" ? 8.0 :
                                        captureMode == "finale" ? 6.0 : captureMode=="resize" ? 2.0 : 1.0;
            if (captureSetting && now - captureStart >= captureDelay) {
                captureFrame(captureMode, width, height);
                glfwSetWindowShouldClose(window_, true);
            }
            if (benchmark) glFinish(); // Include completed GPU work in frame timing.
            glfwSwapBuffers(window_);
            if (benchmark && now-benchmarkStart >= 2.0) frameTimes.push_back(glfwGetTime()-now);
            if (benchmark && now-benchmarkStart >= 17.0) {
                std::ofstream csv(std::string("benchmark_")+benchmarkMode+".csv");
                csv << "frame_ms\n";
                for (double t : frameTimes) csv << std::setprecision(10) << t*1000.0 << '\n';
                const double total = std::accumulate(frameTimes.begin(),frameTimes.end(),0.0);
                std::cout << "Benchmark " << benchmarkMode << ": " << frameTimes.size()/total
                          << " average FPS; frames=" << frameTimes.size() << "\n";
                glfwSetWindowShouldClose(window_,true);
            }
        }

        elapsed += rawDt;
        ++frameCount;
        if (elapsed >= 0.5) {
            fps = float(double(frameCount) / elapsed);
            std::ostringstream title;
            title << "Dream Developer Desk | " << std::fixed
                  << std::setprecision(1) << fps << " FPS"
                  << " | F: finale | N: day/night | P: camera feed";
            glfwSetWindowTitle(window_, title.str().c_str());
            elapsed = 0.0;
            frameCount = 0;
        }
    }
}
