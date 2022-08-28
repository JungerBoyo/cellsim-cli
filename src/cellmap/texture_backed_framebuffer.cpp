//
// Created by reg on 8/10/22.
//
#include "cellmap/texture_backed_framebuffer.hpp"
#include <glad/glad.h>

CSIM::TextureBackedFramebuffer::TextureBackedFramebuffer(std::int32_t width, std::int32_t height)
		: width_(width), height_(height) {
	glCreateTextures(GL_TEXTURE_2D, 1, &texture_2d_id_); // NOLINT single texture
	glTextureStorage2D(texture_2d_id_, 1, GL_RGBA8, width, height);
	glCreateFramebuffers(1, &fbo_id_);
	glNamedFramebufferTexture(fbo_id_, GL_COLOR_ATTACHMENT0, texture_2d_id_, 0);
}
void CSIM::TextureBackedFramebuffer::resize(std::int32_t width, std::int32_t height) {
	width_ = width;
	height_ = height;
	destroy();

	glCreateTextures(GL_TEXTURE_2D, 1, &texture_2d_id_); // NOLINT single texture
	glTextureStorage2D(texture_2d_id_, 1, GL_RGBA8, width, height);
	glCreateFramebuffers(1, &fbo_id_);
	glNamedFramebufferTexture(fbo_id_, GL_COLOR_ATTACHMENT0, texture_2d_id_, 0);
}
void CSIM::TextureBackedFramebuffer::bind_framebuffer() const {
	glBindFramebuffer(GL_FRAMEBUFFER, fbo_id_);
}
void CSIM::TextureBackedFramebuffer::unbind_framebuffer() const {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
void CSIM::TextureBackedFramebuffer::destroy() {
	glDeleteTextures(1, &texture_2d_id_); // NOLINT single texture
	glDeleteFramebuffers(1, &fbo_id_);
}
