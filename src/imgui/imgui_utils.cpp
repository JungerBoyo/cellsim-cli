//
// Created by reg on 8/5/22.
//

#include "imgui/imgui_utils.hpp"
#include "shconfig.hpp"

#include <GLFW/glfw3.h>
#include <exception>
#include <fmt/format.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

void ImGui::Init(void *win) {
	IMGUI_CHECKVERSION();

	const auto glsl_version = fmt::format("#version {}{}0 core", CSIM::shconfig::GLVERSION_MAJOR,
																				CSIM::shconfig::GLVERSION_MINOR);
	if (ImGui::CreateContext() == nullptr) {
		throw std::runtime_error("failed to create imgui context");
	}
	if (!ImGui_ImplGlfw_InitForOpenGL(static_cast<GLFWwindow *>(win), true)) {
		throw std::runtime_error("ImGui_ImplGlfw_InitForOpenGL failed");
	}
	if (!ImGui_ImplOpenGL3_Init(glsl_version.data())) {
		throw std::runtime_error("ImGui_ImplOpenGL3_Init failed");
	}
}

void ImGui::BeginFrameCustom() {
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
}

void ImGui::EndFrameCustom() {
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void ImGui::Uninit() {
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

struct InputTextUserData {
	std::string &str;
	std::int64_t insert_pos;
	bool insert_text;
	std::int32_t clear_pos;
	bool clear_text;
	std::int64_t &cursor_pos;
};

static int InputTextCallback(ImGuiInputTextCallbackData *data) {
	auto *user_data = static_cast<InputTextUserData *>(data->UserData);
	auto &str = user_data->str;
	const auto insert_pos = user_data->insert_pos;

	const auto buf_str_capacity = static_cast<std::size_t>(data->BufSize);
	if (data->EventFlag == ImGuiInputTextFlags_CallbackCharFilter) {
		if (data->EventChar == '\n' || data->EventChar == '>') {
			return 1;
		}
	} else if (data->EventFlag == ImGuiInputTextFlags_CallbackResize) {
		IM_ASSERT(str.data() == data->Buf);
		str.resize(buf_str_capacity);
		data->Buf = str.data();
	}

	if (data->EventFlag == ImGuiInputTextFlags_CallbackAlways) {
		if (user_data->insert_text) {
			auto &str_insert_begin = *std::next(str.begin(), insert_pos);
			data->InsertChars(static_cast<int>(insert_pos), &str_insert_begin);

			constexpr char token = '>';
			bool first_token{false};
			int i = data->BufTextLen;
			for (; i > 0; --i) {
				if (data->Buf[i] == token) { // NOLINT interfacing with imgui
					if (!first_token) {
						first_token = true;
					} else {
						break;
					}
				}
			}

			constexpr int offset{2};
			data->DeleteChars(i + offset, data->BufTextLen - i - offset);
		} else if (user_data->clear_text && user_data->clear_pos <= data->BufTextLen) {
			data->DeleteChars(user_data->clear_pos, data->BufTextLen - user_data->clear_pos);
		}
	}

	user_data->cursor_pos = data->CursorPos;
	return 0;
}

bool ImGui::InputTextMultiline(std::string_view label, std::string &str, bool insert_text,
															 std::int64_t insert_pos, bool clear_text, std::int32_t clear_pos,
															 std::int64_t &cursor_pos, bool readonly) {
	InputTextUserData user_data{str, insert_pos, insert_text, clear_pos, clear_text, cursor_pos};
	auto flags = ImGuiInputTextFlags_CallbackResize |		 // NOLINT imgui
							 ImGuiInputTextFlags_CallbackAlways |		 // NOLINT imgui
							 ImGuiInputTextFlags_CallbackCharFilter; // NOLINT imgui
	if (readonly) {
		flags |= ImGuiInputTextFlags_ReadOnly; // NOLINT imgui
	}

	return InputTextMultiline(label.data(), str.data(), str.size(), {-1, -1}, flags,
														InputTextCallback, &user_data);
}
