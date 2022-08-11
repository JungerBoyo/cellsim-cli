#ifndef CELLSIM_IMGUIUTILS_HPP
#define CELLSIM_IMGUIUTILS_HPP

#include <string>
#include <cinttypes>

namespace ImGui {

void Init(void* win);
void BeginFrameCustom();
void EndFrameCustom();
void Uninit();

/// based on
// https://github.com/ocornut/imgui/blob/master/misc/cpp/imgui_stdlib.cpp
// NOLINTBEGIN

bool InputTextMultiline(std::string_view label, std::string& str, bool insert_text,
												std::int64_t insert_pos, bool clear_text, std::int32_t clear_pos,
												std::int64_t& cursor_pos, bool readonly);
// NOLINTEND
}


#endif
