//
// Created by reg on 8/5/22.
//
#include "cli_emulator/cli_emulator.hpp"

#include <sstream>

#include <imgui.h>
#include <imgui/imgui_utils.hpp>

/**
 * Split string with regard to token. There can be arbitrary number of tokens between strings
 * @example
 * 	word1:token:word2 = word1:token::token: ... :token:word2 = {word1, word2}
 * @param str string to split
 * @param token
 * @return vector of strings
 */
static auto splitString(std::string_view str, char token) {
	std::vector<std::string> result; // result vector of strings
	// ptr to next string to add to result
	const char* str_to_add_ptr = str.front() != ' ' ? str.data() : nullptr;
	std::size_t str_to_add_i = 0; // beginning position of next string to add result
	bool tokens = false; // true if tokens strike is on turns to false after adding next string
	for(std::size_t i=0; i<str.length(); ++i)	{
		std::size_t first_token_i = 0;
		for(;i<str.length() && str.at(i) == token; ++i){
			if(!tokens) {
				first_token_i = i;
				tokens = true;
			}
		}
		if(tokens) {
			if(str_to_add_ptr != nullptr) {
				result.emplace_back(str_to_add_ptr, first_token_i - str_to_add_i);
			}
			if(i < str.length()) {
				str_to_add_ptr = &str.at(i);
				str_to_add_i = i;
				tokens = false;
			}
		}
	}

	if (!result.empty()) {
		if (!tokens) {
			result.emplace_back(str_to_add_ptr);
		}
	} else {
		result.emplace_back(str);
	}

	return result;
}

bool CSIM::CLIEmulator::draw(bool parse, bool backspace) {
	bool result{ false };
	/// decrement counter and check if enter was pressed (parse)
	if (just_parsed_counter_ = just_parsed_counter_ > 0 ? just_parsed_counter_ - 1 : 0;
			parse && just_parsed_counter_ == 0)	{
		just_parsed_counter_ = 60; // NOLINT next parse possible in ~1 second

		const auto unsigned_next_insert_pos = static_cast<std::size_t>(next_insert_pos_);
		/// get index of end termination char
		const auto end_index = static_cast<std::int64_t>(
				cli_text_.find_first_of('\0', unsigned_next_insert_pos));

		/// if there is anything to parse
		if(end_index > blocking_cursor_pos_) {
			result = true;
			/// construct parse string taken from the cli buffer
			const std::string_view parse_str {
					std::next(cli_text_.cbegin(), next_insert_pos_),
					std::next(cli_text_.cbegin(), end_index)};
			/// parse string and transform to c style pointer
			const auto args_str = splitString(parse_str, ' ');
			std::vector<const char*> args(args_str.size());
			std::transform(args_str.cbegin(), args_str.cend(), args.begin(),
										 [](const std::string& str) {
											 return str.c_str();
										 });
			/// parse arguments with CLI11 app parser
			std::stringstream parser_output;
			try { parser.parse(static_cast<int>(args.size()), args.data()); }
			catch (const CLI::ParseError& e) {
				const auto e_name = e.get_name();
				result = e.get_exit_code() == static_cast<int>(CLI::ExitCodes::Success) &&
						     e_name != "CallForHelp" && e_name != "CallForAllHelp" &&
								 e_name != "CallForVersion";
				parser.exit(e, parser_output, parser_output);
			}
			/// if there ve been any output to stream
			if(parser_output.rdbuf()->is_avail() > 0) {
				/// copy the contents of the parser output into a cli buffer
				const auto parser_output_str = "\n" + parser_output.str();
				const auto parser_output_str_len_diff = static_cast<std::int64_t>(parse_str.length());
				std::copy(parser_output_str.cbegin(), parser_output_str.cend(),
									std::next(cli_text_.begin(), next_insert_pos_ + parser_output_str_len_diff));
				/// add new prompt
				const auto prompt_len_diff =
						static_cast<std::int64_t>(parser_output_str.length());
				std::copy(prompt_.cbegin(), prompt_.cend(),
									std::next(cli_text_.begin(),
														next_insert_pos_ + parser_output_str_len_diff + prompt_len_diff));
				/// set next output/insert position
				next_insert_pos_ += static_cast<std::int64_t>(parse_str.length() +
																											parser_output_str.length());
				/// move readonly area further
				blocking_cursor_pos_ = next_insert_pos_ + static_cast<std::int64_t>(prompt_.length()) - 1;
			} else { /// if there wasn't any output just construct new prompt
				const auto prompt_str = std::string(prompt_);
				const auto next_prompt = "\n" + prompt_str;
				std::copy(next_prompt.cbegin(), next_prompt.cend(),
									std::next(cli_text_.begin(), next_insert_pos_ +
																							 static_cast<std::int64_t>(parse_str.length())));

				next_insert_pos_ += static_cast<std::int64_t>(parse_str.length());
				blocking_cursor_pos_ = next_insert_pos_ + static_cast<std::int64_t>(prompt_.length());
			}
		}

		cli_text_changed_ = true;
	}

	ImGui::Begin(label_.c_str(), nullptr, 0);
	ImGui::InputTextMultiline("##source", cli_text_, cli_text_changed_, current_insert_pos_,
														clear_text_, static_cast<std::int32_t>(prompt_.length())-1, cursor_pos_,
														/// if ~ENTER and BACKSPACE and cursor is <= blocking position
														/// then switch to readonly mode
														!parse && backspace && cursor_pos_ <= blocking_cursor_pos_);
	ImGui::End();

	if (cli_text_changed_) {
		current_insert_pos_ = next_insert_pos_;
		cli_text_changed_ = false;
	}
	clear_text_ = false;

	return result;
}
void CSIM::CLIEmulator::clear() {
	current_insert_pos_ = next_insert_pos_ = 0;
	blocking_cursor_pos_ = static_cast<std::int64_t>(prompt_.length()) - 1;
	std::fill(cli_text_.begin(), cli_text_.end(), '\0');
	cli_text_.resize(prompt_.length());
	std::copy(prompt_.cbegin(), prompt_.cend(), cli_text_.begin());
	clear_text_ = true;
}
