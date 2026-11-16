#include "graphics/ShadowMap.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <stdexcept>

ShadowMap::ShadowMap() {
    glGenFramebuffers(1, &framebuffer_);
    glGenTextures(1, &depth_);
    glBindTexture(GL_TEXTURE_2D, depth_);
    glTexImage2D(GL_TEXTURE_2D,0,GL_DEPTH_COMPONENT24,resolution,resolution,0,GL_DEPTH_COMPONENT,GL_FLOAT,nullptr);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_BORDER);
    const float border[] = {1,1,1,1};
    glTexParameterfv(GL_TEXTURE_2D,GL_TEXTURE_BORDER_COLOR,border);
    glBindFramebuffer(GL_FRAMEBUFFER,framebuffer_);
    glFramebufferTexture2D(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_TEXTURE_2D,depth_,0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    const bool complete = glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
    glBindFramebuffer(GL_FRAMEBUFFER,0);
    glBindTexture(GL_TEXTURE_2D,0);
    if (!complete) {
        glDeleteFramebuffers(1,&framebuffer_);
        glDeleteTextures(1,&depth_);
        throw std::runtime_error("Shadow framebuffer incomplete");
    }
}
ShadowMap::~ShadowMap() {
    glDeleteFramebuffers(1,&framebuffer_);
    glDeleteTextures(1,&depth_);
}
void ShadowMap::begin() const {
    glBindFramebuffer(GL_FRAMEBUFFER,framebuffer_);
    glViewport(0,0,resolution,resolution);
    glClear(GL_DEPTH_BUFFER_BIT);
}
void ShadowMap::bindTexture(unsigned int unit) const {
    glActiveTexture(GL_TEXTURE0+unit);
    glBindTexture(GL_TEXTURE_2D,depth_);
    glActiveTexture(GL_TEXTURE0);
}
glm::mat4 ShadowMap::lightMatrix() {
    return glm::ortho(-8.f,8.f,-8.f,8.f,1.f,24.f) *
        glm::lookAt(glm::vec3(-3,8,5),glm::vec3(0,0,0),glm::vec3(0,1,0));
}
