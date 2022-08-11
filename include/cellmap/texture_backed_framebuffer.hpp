//
// Created by reg on 8/10/22.
//

#ifndef CELLSIM_TEXTURE_BACKED_FRAMEBUFFER_HPP
#define CELLSIM_TEXTURE_BACKED_FRAMEBUFFER_HPP

#include <cinttypes>

namespace CSIM {

struct TextureBackedFramebuffer{
	std::int32_t width_;
	std::int32_t height_;
	std::uint32_t texture_2d_id_{0};
	std::uint32_t fbo_id_{0};

	TextureBackedFramebuffer(std::int32_t width, std::int32_t height);
	void resize(std::int32_t width, std::int32_t height);

	void bind_framebuffer() const;
	void unbind_framebuffer() const;

	[[nodiscard]] auto tex_id() const { return texture_2d_id_; }

	[[nodiscard]]	auto width() const { return width_; }
	[[nodiscard]]	auto height() const { return height_; }

	void destroy();
};

}

#endif // CELLSIM_TEXTURE_BACKED_FRAMEBUFFER_HPP
