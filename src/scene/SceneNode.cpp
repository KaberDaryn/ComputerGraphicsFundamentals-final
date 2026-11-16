#include "scene/SceneNode.hpp"
#include "graphics/Mesh.hpp"
#include "graphics/Shader.hpp"
#include "graphics/Texture.hpp"
#include <utility>

SceneNode::SceneNode(std::string name) : name_(std::move(name)) {}
SceneNode& SceneNode::addChild(std::unique_ptr<SceneNode> child) {
    child->parent_ = this;
    children_.push_back(std::move(child));
    return *children_.back();
}
SceneNode* SceneNode::findByName(const std::string& name) {
    if (name_ == name) return this;
    for (auto& child : children_) {
        if (auto* result = child->findByName(name)) return result;
    }
    return nullptr;
}
glm::mat4 SceneNode::worldTransform() const {
    return parent_ ? parent_->worldTransform() * localTransform : localTransform;
}
glm::vec3 SceneNode::worldPosition() const {
    return glm::vec3(worldTransform() * glm::vec4(0, 0, 0, 1));
}
void SceneNode::draw(const Shader& shader, const glm::mat4& parent, RenderPass pass) const {
    // The transform of a child is relative to its parent's coordinate system.
    const glm::mat4 world = parent * localTransform;
    const bool shadow = pass == RenderPass::Shadow && castsShadow &&
                        (renderPass == RenderPass::Lit || renderPass == RenderPass::RGB);
    if (mesh && (renderPass == pass || shadow)) {
        shader.setMat4("uModel", world);
        if (pass == RenderPass::RGB) {
            shader.setFloat("uRgbPhase", rgbPhase);
        } else if (pass == RenderPass::Lit || pass == RenderPass::Glass) {
            shader.setVec3("uBaseColor", color);
            shader.setVec3("uEmission", emission);
            shader.setFloat("uShininess", shininess);
            shader.setFloat("uOpacity", opacity);
            shader.setVec2("uUvScale", uvScale);
            shader.setBool("uUseTexture", texture != nullptr);
            shader.setBool("uUnlit", unlit);
            if (texture) texture->bind(0);
        }
        mesh->draw();
    }
    for (const auto& child : children_) child->draw(shader, world, pass);
}
