//
// Created by reg on 8/5/22.
//

#ifndef CELLSIM_WINDOW_HPP
#define CELLSIM_WINDOW_HPP

#include <cinttypes>
#include <functional>
#include <memory>
#include <string_view>

namespace CSIM {
struct Window {
	struct UserData;
	struct WinNative;

	std::shared_ptr<WinNative> win_handle_;
	std::unique_ptr<UserData, void (*)(UserData *)> user_data_;

	static void initWindowingSystem();
	static void uninitWindowingSystem();

	void loadGL();
	void *native();

	Window(std::int32_t width, std::int32_t height, std::string_view title,
				 void (*error_callback)(int, const char *) = nullptr);

	void destroy();

	void setScrollCallback(const std::function<void(float)> &callback);
	void setCursorPosCallback(const std::function<void(float, float, float, float)> &callback);

	int getKeyState(int key_code);
	int getButtonState(int button_code);
	float getTime();
	std::tuple<int, int> getWindowSize();
	std::tuple<float, float> getMousePos();

	bool shouldClose();
	void pollEvents();
	void swapBuffers();
};
} // namespace CSIM

#endif // CELLSIM_WINDOW_HPP
