#include "scene/SceneFactory.hpp"
#include "scene/RoomLayout.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>

namespace SceneFactory {
void buildRoom(SceneNode& root, const SceneAssets& a) {
    using namespace RoomLayout;
    // The side wall owns the corner square. The back wall meets its inner face;
    // neither wall crosses the floor boundary or protrudes through the other.
    part(root,"Floor platform",a.cube,
         {centerX,floorY-platformThickness*.5f-.001f,centerZ},
         {width,platformThickness,depth},{.48f,.36f,.25f},nullptr,{1,1},10);
    part(root, "Oak floor", a.plane, {centerX,floorY,centerZ}, {width,1,depth},
         {1.0f, 0.94f, 0.85f}, a.floor, {3, 3}, 12).castsShadow=false;
    part(root, "Back wall", a.cube,
         {(left+right)*.5f,floorY+wallHeight*.5f,back-wallThickness*.5f},
         {right-left,wallHeight,wallThickness},
         {0.77f, 0.75f, 0.68f}, nullptr, {1, 1}, 5);
    part(root, "Side wall", a.cube,
         {left-wallThickness*.5f,floorY+wallHeight*.5f,centerZ},
         {wallThickness,wallHeight,depth},
         {0.48f, 0.58f, 0.55f}, nullptr, {1, 1}, 5);
    part(root, "Baseboard", a.cube,
         {(left+skirtDepth+right)*.5f,floorY+skirtHeight*.5f,back+skirtDepth*.5f},
         {right-left-skirtDepth,skirtHeight,skirtDepth},
         {0.41f, 0.30f, 0.21f}, a.wood, {5, 1}, 12);
    part(root, "Side oak baseboard", a.cube,
         {left+skirtDepth*.5f,floorY+skirtHeight*.5f,(back+front)*.5f},
         {skirtDepth,skirtHeight,front-back},
         {0.41f, 0.30f, 0.21f}, a.wood, {1, 5}, 12);

    // The official flag image faces +Z. A single quad keeps its UVs unmirrored;
    // its horizontal/vertical ratio is exactly 2:1, with no repeat or crop.
    auto& flag = group(root, "Kazakhstan wall flag", {-0.05f, 3.34f, -3.31f});
    part(flag, "Flag fabric backing", a.cube, {0, 0, -0.018f},
         {2.20f, 1.10f, 0.025f}, {0.025f, 0.48f, 0.58f}, nullptr, {1, 1}, 3);
    part(flag, "Official Kazakhstan flag", a.screen, {0, 0, 0},
         {2.20f, 1.10f, 1}, {1, 1, 1}, a.flag, {1, 1}, 3);
    for (float x : {-1.06f, 1.06f})
        part(flag, "Flag brass mounting pin", a.sphere, {x, 0.51f, 0.012f},
             {0.032f, 0.032f, 0.018f}, {0.70f, 0.52f, 0.23f}, nullptr, {1, 1}, 55);

    // Procedural sky behind the window frame; day/night is driven by GLSL.
    auto& window = group(root, "Window frame", {2.50f, 2.69f, -3.365f});
    auto& sky = part(window, "Window sky gradient", a.cube, {0, 0, 0.045f},
                     {1.95f, 1.35f, 0.026f}, {1, 1, 1});
    sky.renderPass = RenderPass::Sky;
    for (float side : {-1.0f, 1.0f}) {
        part(window, "Frame vertical", a.cube, {side * 1.01f, 0, 0.10f},
             {0.09f, 1.50f, 0.10f}, {0.08f, 0.11f, 0.15f});
        part(window, "Frame horizontal", a.cube, {0, side * 0.72f, 0.10f},
             {2.12f, 0.085f, 0.10f}, {0.08f, 0.11f, 0.15f});
    }
    part(window, "Window mullion", a.cube, {0, 0, 0.108f},
         {0.07f, 1.40f, 0.08f}, {0.08f, 0.11f, 0.15f});
    part(window, "Window crossbar", a.cube, {0, 0.0f, 0.108f},
         {2.0f, 0.055f, 0.08f}, {0.08f, 0.11f, 0.15f});
    part(window, "Window sill", a.cube, {0, -0.78f, 0.28f},
         {2.28f, 0.10f, 0.40f}, {0.69f, 0.66f, 0.66f});

    part(window, "Curtain oak rail", a.cube, {0, 0.96f, 0.21f},
         {3.0f, 0.055f, 0.065f}, {0.35f, 0.25f, 0.17f}, a.wood, {2, 1}, 12);
    // Alternating depth and colour suggest linen folds without transparency.
    for (float side : {-1.0f, 1.0f}) for (int fold = 0; fold < 4; ++fold)
        part(window, "Linen curtain fold", a.cube,
             {side * (1.13f + fold * 0.085f), -0.02f, 0.22f + (fold % 2) * 0.035f},
             {0.10f, 1.86f, 0.075f},
             fold % 2 ? glm::vec3(0.72f, 0.65f, 0.53f) : glm::vec3(0.86f, 0.79f, 0.66f),
             nullptr, {1, 1}, 2);

    auto& shelf = group(root, "Wall shelves", {-2.55f, 2.65f, -3.10f});
    for (int r = 0; r < 2; ++r) {
        const float y = 0.30f - float(r) * 0.72f;
        part(shelf, "Wood shelf", a.cube, {0, y, 0}, {1.75f, 0.09f, 0.38f},
             {0.83f, 0.74f, 0.62f}, a.wood);
        for (int b = 0; b < 5; ++b) {
            float height = 0.23f + 0.04f * float(b % 3);
            part(shelf, "Book spine", a.cube,
                 {-0.64f + float(b) * 0.18f, y + height * 0.5f + 0.045f, 0},
                 {0.11f, height, 0.27f},
                 b % 3 == 0 ? glm::vec3(0.52f, 0.24f, 0.16f) :
                 b % 3 == 1 ? glm::vec3(0.20f, 0.39f, 0.38f) : glm::vec3(0.77f, 0.65f, 0.44f),
                 nullptr, {1, 1}, 6);
        }
        part(shelf, "Ceramic shelf vase", a.sphere, {0.53f, y + 0.19f, 0},
             {0.24f, 0.29f, 0.23f}, {0.70f, 0.53f, 0.37f}, nullptr, {1, 1}, 12);
        part(shelf, "Vase neck", a.cylinder, {0.53f, y + 0.34f, 0},
             {0.095f, 0.12f, 0.095f}, {0.70f, 0.53f, 0.37f}, nullptr, {1, 1}, 12);
    }

    // Fabric only on the upper plane: cube UVs would duplicate the motif on
    // the edges. Both surfaces clear the floor and the chair wheels.
    part(root, "Rug woven edge", a.cube, {0, -0.002f, 1.55f}, {3.80f, 0.022f, 3.35f},
         {0.40f, 0.30f, 0.22f}, nullptr, {1, 1}, 2);
    part(root, "Kazakh inspired woven rug", a.plane, {0, 0.011f, 1.55f}, {3.80f, 1, 3.35f},
         {1, 1, 1}, a.rug, {1, 1}, 2);

    auto& plant = group(root, "Floor rubber plant", {-2.55f, 0, -0.25f});
    part(plant, "Terracotta floor planter", a.cylinder, {0, 0.28f, 0},
         {0.62f, 0.55f, 0.62f}, {0.64f, 0.32f, 0.20f}, nullptr, {1, 1}, 9);
    part(plant, "Planter soil", a.cylinder, {0, 0.56f, 0},
         {0.53f, 0.015f, 0.53f}, {0.13f, 0.10f, 0.07f}, nullptr, {1, 1}, 2);
    part(plant, "Plant trunk", a.cylinder, {0, 1.02f, 0},
         {0.045f, 1.0f, 0.045f}, {0.27f, 0.22f, 0.12f}, nullptr, {1, 1}, 5);
    for (int i = 0; i < 7; ++i) {
        const float angle = i * 2.40f;
        auto& leaf = part(plant, "Rubber plant leaf", a.sphere,
             {0.23f * std::cos(angle), 0.86f + i * 0.105f, 0.23f * std::sin(angle)},
             {0.48f, 0.09f, 0.24f},
             i % 2 ? glm::vec3(0.16f, 0.32f, 0.17f) : glm::vec3(0.23f, 0.42f, 0.22f),
             nullptr, {1, 1}, 12);
        // Rotation precedes scale to keep leaves flat and radial.
        leaf.localTransform = glm::translate(glm::mat4(1),
            {0.23f * std::cos(angle), 0.86f + i * 0.105f, 0.23f * std::sin(angle)}) *
            glm::rotate(glm::mat4(1), -angle, glm::vec3(0, 1, 0)) *
            glm::rotate(glm::mat4(1), 0.28f, glm::vec3(0, 0, 1)) *
            glm::scale(glm::mat4(1), glm::vec3(0.48f, 0.09f, 0.24f));
    }
}
}
