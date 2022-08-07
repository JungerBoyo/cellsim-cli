//
// Created by reg on 8/5/22.
//

#ifndef CELLSIM_CLI_EMULATOR_HPP
#define CELLSIM_CLI_EMULATOR_HPP

#include <string_view>
#include <sstream>
#include <CLI/CLI.hpp>

namespace CSIM {

/** CLI emulator class
 * Provides abstraction of ImGui based cli. Inherit from this class and override
 * setCLI method to set options. Provides CLI11 app field through which it is possible
 * to define subcommands and options.
 * @tparam CLIConfig_t type of config struct to use inside setCLI virtual method
 */
struct CLIEmulator {
	static constexpr std::size_t INITIAL_CLI_BUF_SIZE { 4096 }; /*!< initial capacity of input text buffer */
protected:
	CLI::App parser; /*!< CLI11 app */

private:
	std::string prompt_; /*!< prompt that will be shown in a CLI @note  */
	std::string label_; /*!< title of cli window */

	std::string cli_text_{ prompt_ }; /*!< cli text buffer */

	std::int32_t just_parsed_counter_{ 0 }; /*!< counter preventing from false enter key press spam*/
	std::stringstream parser_output_; /*!< output of the CLI11 parse */
	std::int64_t current_insert_pos_{ 0 }; /*!< current cursor insert cli output position */
	std::int64_t next_insert_pos_{ 0 }; /*!< next cursor insert cli output position */
	//! cursor position before which the edit is disabled
	std::int64_t blocking_cursor_pos_{ static_cast<std::int64_t>(prompt_.size()) - 1 };
	std::int64_t cursor_pos_ { 0 }; /*!< current cursor position (set by imgui callback) */
	bool cli_text_changed_{ false }; /*!< info to ImGui about text change */
public:
	/**
	 * CLI emulator constructor
	 * @param title maps to label member field, will be displayed in cli window tab add ## at the
	 * front for it to not be displayed.
	 * @param description description of CLI11 app.
	 * @param prompt text that will be prompt in a CLI. '>' char will be appended.
	 */
	explicit CLIEmulator(std::string_view title, std::string_view description, std::string_view prompt)
			: parser(description.data()), prompt_(std::string(prompt) + ">  "), label_(title)  {
		cli_text_.reserve(INITIAL_CLI_BUF_SIZE);
	}
	/**
	 * Method draws CLI and based on passed parameters parse the current command
	 * @param parse tells if ENTER key was pressed.
	 * @param backspace tells if BACKSPACE key was pressed.
	 * @return if haven't parsed anything that can be processed by a user FALSE\n
	 * 				 else TRUE (app CLI11 new arguments can be processed by a user)
	 */
	bool draw(bool parse, bool backspace);

	/**
	 * virtual method to override and set CLI options inside thorugh CLI11 app aka parse
	 * @param cli_config config with subcommand/options data bindings to add
	 */
	virtual void setCLI() = 0;

	virtual ~CLIEmulator() = default;
};
}

#endif // CELLSIM_CLI_EMULATOR_HPP
