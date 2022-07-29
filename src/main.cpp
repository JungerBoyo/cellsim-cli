#include <hello_world/hello_world.hpp>

#include <stdexcept>
#include <vector>
#include <optional>
#include <filesystem>
#include <fstream>
#include <string_view>
#include <memory>
#include <array>
#include <span>
#include <algorithm>

#include <fmt/core.h>
#include <CLI/CLI.hpp>
#include <spdlog/spdlog.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

// NOLINTBEGIN
void glDebugCallback(
  GLenum, 
  GLenum, 
  GLuint id, 
  GLenum severity, 
  GLsizei, 
  const GLchar* message,
  const void*  
) {
  if(severity == GL_DEBUG_SEVERITY_HIGH) {
    spdlog::error("[glad] {}, id = {}", message, id);
  } else {
    spdlog::info("[glad] {}, id = {}", message, id);
  }
}
// NOLINTEND

constexpr std::int32_t GLVERSION_MAJOR{ 4 };
constexpr std::int32_t GLVERSION_MINOR{ 5 };
constexpr GLuint NULL_BUFFER_ID{ 0 };
constexpr GLuint IN_POSITION_LOCATION{ 0 };
constexpr GLuint VIEW_CONFIG_UBO_BINDING_LOCATION{ 0 };
constexpr GLuint COLORS_UBO_BINDING_LOCATION{ 1 };
constexpr GLuint STEP_SHADER_CONFIG_UBO_BINDING_LOCATION{ 1 };
constexpr GLuint STEP_SHADER_BASE_CONFIG_UBO_BINDING_LOCATION{ 0 };
constexpr GLuint STATE_MAP_SSBO_BINDING_LOCATION{ 2 };
constexpr GLuint INSTANCE_OFFSETS_SSBO_BINDING_LOCATION{ 3 };
constexpr std::size_t MAX_COLORS = 256;
constexpr std::string_view VSH_RENDER_SHADER_PATH = "shaders/bin/render_shader/vert.spv";
constexpr std::string_view FSH_RENDER_SHADER_PATH = "shaders/bin/render_shader/frag.spv";

template<typename T> requires std::is_arithmetic<T>::value
struct Vec4 {
	T x = static_cast<T>(0);
	T y = static_cast<T>(0);
	T z = static_cast<T>(0);
	T w = static_cast<T>(0);
};

template<typename T> requires std::is_arithmetic<T>::value
struct Vec2 {
	T x = static_cast<T>(0);
	T y = static_cast<T>(0);
};

namespace CA {

struct Shader { // NOLINT no need for move constructors
  GLuint program_id_;
    
  Shader() : program_id_(glCreateProgram()) {}

  void bind() const noexcept {
    glUseProgram(program_id_);
  }

  static void unbind() {
    glUseProgram(0);
  }

  static std::vector<char> parseShader(const std::filesystem::path& path) {
    std::ifstream stream(path.c_str(), std::ios::binary|std::ios::ate);

    if(!stream.good()) {
      throw std::runtime_error(fmt::format("failed to create stream from shader file {}", path.c_str()));
    }

    const auto size = static_cast<std::size_t>(stream.tellg());
    std::vector<char> code(size);

    stream.seekg(0);
    stream.read(code.data(), static_cast<std::streamsize>(size));

    stream.close();

    return code;
  }

  virtual void destroy() noexcept {
    glDeleteProgram(program_id_);
  }

  virtual ~Shader() = default; 
};

struct VFShader : public Shader {
  GLuint vsh_id_;
  GLuint fsh_id_;

  VFShader(
    const std::filesystem::path& vsh_path, 
    const std::filesystem::path& fsh_path)
      : vsh_id_(glCreateShader(GL_VERTEX_SHADER)), fsh_id_(glCreateShader(GL_FRAGMENT_SHADER)) {
    const auto vsh_binary = parseShader(vsh_path);
    const auto fsh_binary = parseShader(fsh_path);

    glShaderBinary(1, &vsh_id_, GL_SHADER_BINARY_FORMAT_SPIR_V_ARB, vsh_binary.data(), static_cast<GLsizei>(vsh_binary.size()));
    glShaderBinary(1, &fsh_id_, GL_SHADER_BINARY_FORMAT_SPIR_V_ARB, fsh_binary.data(), static_cast<GLsizei>(fsh_binary.size()));
     
    glSpecializeShaderARB(vsh_id_, "main", 0, nullptr, nullptr);
    glSpecializeShaderARB(fsh_id_, "main", 0, nullptr, nullptr);   

    glAttachShader(this->program_id_, vsh_id_);
    glAttachShader(this->program_id_, fsh_id_);

    glValidateProgram(this->program_id_);
    glLinkProgram(this->program_id_);
  }

  void destroy() noexcept override {
    glDetachShader(this->program_id_, vsh_id_);
    glDetachShader(this->program_id_, fsh_id_);

    glDeleteShader(vsh_id_);
    glDeleteShader(fsh_id_);

    Shader::destroy();
  }
};


struct CShader : public Shader {
  GLuint csh_id_;

  explicit CShader(const std::filesystem::path& csh_path) 
    : csh_id_(glCreateShader(GL_COMPUTE_SHADER)) {
    const auto csh_binary = parseShader(csh_path);
    
    glShaderBinary(1, &csh_id_, GL_SHADER_BINARY_FORMAT_SPIR_V_ARB, csh_binary.data(), static_cast<GLsizei>(csh_binary.size()));
   
    glSpecializeShaderARB(csh_id_, "main", 0, nullptr, nullptr); 

    glAttachShader(this->program_id_, csh_id_);

    glValidateProgram(this->program_id_);
    glLinkProgram(this->program_id_);
  } 

	void destroy() noexcept override {
    glDetachShader(this->program_id_, csh_id_);

    glDeleteShader(csh_id_);

    Shader::destroy();
  }
};


struct CellMap {
  GLuint instance_offsets_ssbo_id_{ NULL_BUFFER_ID };
  GLuint state_map_ssbo_id_       { NULL_BUFFER_ID };

	std::shared_ptr<const Shader> render_shader_{ nullptr };

  std::size_t width_;
  std::size_t height_; 
  std::vector<Vec2<float>> cell_offsets_;
  std::vector<std::int32_t> cell_states_;
  std::int32_t state_count_;
  static constexpr std::int32_t INITIAL_LIFE_STATE{ 1 };

  CellMap(std::shared_ptr<const Shader> render_shader,
					std::size_t width,
					std::size_t height,
					std::int32_t state_count)
    : render_shader_(std::move(render_shader)),
				width_(width), height_(height), cell_offsets_(width * height),
				cell_states_(width * height, 0), state_count_(state_count) {
    generateOffsets();

		std::array<GLuint, 2> buffers; //NOLINT
		glCreateBuffers(buffers.size(), buffers.data());
		instance_offsets_ssbo_id_ = buffers[0];
		state_map_ssbo_id_        = buffers[1];

		// set up ssbos
		render_shader_->bind();

		glNamedBufferStorage(instance_offsets_ssbo_id_, static_cast<GLsizei>( cell_offsets_.size() * sizeof(Vec2<float>) ), cell_offsets_.data(), 0); // NOLINT no flags = readonly
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, INSTANCE_OFFSETS_SSBO_BINDING_LOCATION, instance_offsets_ssbo_id_);

		glNamedBufferStorage(state_map_ssbo_id_, static_cast<GLsizei>( cell_states_.size() * sizeof(std::int32_t) ), cell_states_.data(), GL_DYNAMIC_STORAGE_BIT);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, STATE_MAP_SSBO_BINDING_LOCATION, state_map_ssbo_id_);

		render_shader_->unbind();
	}

	void seed(std::size_t x, std::size_t y) noexcept {
    cell_states_.at(x + y * width_) = INITIAL_LIFE_STATE;
  }

  void generateOffsets() noexcept {
    for(auto y=0ul; y<height_; ++y) {
      for(auto x=0ul; x<width_; ++x) {
        cell_offsets_.at(x + y*width_) = {static_cast<float>(x), static_cast<float>(y)};
      }
    }
  }

	void destroy() noexcept {
		std::array<GLuint, 2> buffers; // NOLINT
		buffers[0] = instance_offsets_ssbo_id_;
		buffers[1] = state_map_ssbo_id_;

		glDeleteBuffers(buffers.size(), buffers.data());
	};
};

struct Renderer {
  GLuint vbo_id_                  { NULL_BUFFER_ID };
  GLuint vao_id_                  { NULL_BUFFER_ID };
  GLuint view_config_ubo_id_      { NULL_BUFFER_ID };
  GLuint colors_ubo_id_           { NULL_BUFFER_ID };

  std::shared_ptr<const Shader> render_shader_{ nullptr };
	float time_step_{ 0.f };
  
  struct ViewConfig {
    Vec2<float> offset;
    float scale;
  }; 

  ViewConfig view_config_{{}, 1.f};
  std::array<Vec4<float>, MAX_COLORS> colors_;

	static constexpr std::array<float, 8> QUAD {{
		-.5f, -.5f, .5f, -.5f, .5f, .5f, -.5f, .5f
	}};

	explicit Renderer(std::shared_ptr<const Shader> render_shader)
    : render_shader_( std::move(render_shader) ){

    std::array<GLuint, 3> buffers; // NOLINT initialization through ptr
    glCreateBuffers(buffers.size(), buffers.data());
    vbo_id_                   = buffers[0];
    view_config_ubo_id_       = buffers[1];
    colors_ubo_id_            = buffers[2];

    // set up vertex data
    glNamedBufferStorage(vbo_id_, QUAD.size() * sizeof(float), QUAD.data(), 0);// NOLINT no flags = readonly data

    // specify attrib layout 
    glCreateVertexArrays(1, &vao_id_); //NOLINT single vao
    glBindVertexArray(vao_id_);
		glBindBuffer(GL_ARRAY_BUFFER, vbo_id_);
    
    glVertexAttribPointer(IN_POSITION_LOCATION, 2, GL_FLOAT, GL_FALSE, 0, nullptr);// NOLINT stride=0, type=fvec2
		glEnableVertexAttribArray(IN_POSITION_LOCATION);

		glBindBuffer(GL_ARRAY_BUFFER, NULL_BUFFER_ID);
    glBindVertexArray(0);

    // set up ubos for ViewConfig and Colors
		render_shader_->bind();
    
    glNamedBufferStorage(view_config_ubo_id_, sizeof(ViewConfig), &view_config_, GL_DYNAMIC_STORAGE_BIT);
    glBindBufferBase(GL_UNIFORM_BUFFER, VIEW_CONFIG_UBO_BINDING_LOCATION, view_config_ubo_id_);

    glNamedBufferStorage(colors_ubo_id_, static_cast<GLsizei>( colors_.size() * sizeof(Vec4<float>) ), colors_.data(), GL_DYNAMIC_STORAGE_BIT);
    glBindBufferBase(GL_UNIFORM_BUFFER, COLORS_UBO_BINDING_LOCATION, colors_ubo_id_);

		render_shader_->unbind();
  }

	void setColors(std::span<Vec4<float>> colors) {
		const std::size_t colors_size = std::clamp(colors.size(), 0ul, MAX_COLORS-1);
		std::copy_n(colors.begin(), colors_size, colors_.begin());

		glNamedBufferSubData(colors_ubo_id_, 0, static_cast<GLsizeiptr>(sizeof(Vec4<float>) * colors_size), colors_.data());
	}

	void updateView(Vec2<float> offset_vec, float scale_vec)	noexcept {
		const auto step = 1.f + time_step_;
		view_config_ = {
				{step * offset_vec.x, step * offset_vec.y},
				step * scale_vec
		};
	}

  void draw(const CellMap& cell_map) noexcept {
		render_shader_->bind();
		glBindVertexArray(vao_id_);
		glBindBuffer(GL_ARRAY_BUFFER, vbo_id_);

		glNamedBufferSubData(view_config_ubo_id_, 0, sizeof(ViewConfig), &view_config_);

		// inserting memory barrier for a shader storages because of the state_map
		// which is modified in step shaders of a rules
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
		glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, QUAD.size()/2, static_cast<GLsizei>(cell_map.cell_offsets_.size()));// NOLINT draw QUAD from triangles

		glBindBuffer(GL_ARRAY_BUFFER, NULL_BUFFER_ID);
		glBindVertexArray(0);
		render_shader_->unbind();
  }

  void destroy() { 
    std::vector<GLuint> buffers;  
    buffers.reserve(3); //NOLINT
    
    buffers.push_back(vbo_id_);     
    buffers.push_back(view_config_ubo_id_);
    buffers.push_back(colors_ubo_id_);
  }
};

struct RuleConfig { // NOLINT no need for move constructor/assignment
	std::shared_ptr<Shader> step_shader_;

	explicit RuleConfig(std::shared_ptr<Shader> step_shader)
			: step_shader_(std::move(step_shader)) {}

	[[nodiscard]] virtual std::size_t size() const = 0;
	[[nodiscard]] virtual const void* data() const = 0;

	[[nodiscard]] std::shared_ptr<const Shader> getShader() const { return step_shader_; }

	virtual void destroy() {
		step_shader_->destroy();
	}

	virtual ~RuleConfig() = default;
};

struct Rule { // NOLINT no need for move constructor/assignment
	GLuint base_config_ubo_id_ { NULL_BUFFER_ID };
	GLuint config_ubo_id_			 { NULL_BUFFER_ID };

	struct BaseConfig {
		Vec2<std::int32_t> map_resolution;
		std::int32_t state_count;
		std::int32_t iteration;
	};

	BaseConfig base_config_;
	std::shared_ptr<RuleConfig> rule_config_;

	Rule(Vec2<std::int32_t> map_resolution, std::int32_t state_count,
			 std::shared_ptr<RuleConfig> rule_config)
			: base_config_{map_resolution, state_count, 0},
				rule_config_(std::move(rule_config)) {

		std::array<GLuint, 2> buffers; // NOLINT initialization through ptr
		glCreateBuffers(buffers.size(), buffers.data());
		base_config_ubo_id_  = buffers[0];
		config_ubo_id_       = buffers[1];

		glNamedBufferStorage(base_config_ubo_id_, sizeof(BaseConfig), &base_config_,
												 GL_DYNAMIC_STORAGE_BIT);
		glNamedBufferStorage(config_ubo_id_, sizeof(rule_config_->size()),
												 rule_config_->data(), GL_DYNAMIC_STORAGE_BIT);

		rule_config_->getShader()->bind();
		glBindBufferBase(GL_UNIFORM_BUFFER, STEP_SHADER_BASE_CONFIG_UBO_BINDING_LOCATION, base_config_ubo_id_);
		glBindBufferBase(GL_UNIFORM_BUFFER, STEP_SHADER_CONFIG_UBO_BINDING_LOCATION, config_ubo_id_);
		rule_config_->getShader()->unbind();
	}

	virtual void destroy() {
		std::array<GLuint, 2> buffers; // NOLINT
		buffers[0] = base_config_ubo_id_;
		buffers[1] = config_ubo_id_;

		glDeleteBuffers(buffers.size(), buffers.data());
	}

	virtual void step(const CellMap& cell_map) = 0;
	virtual ~Rule() = default;
};
/*
 * TODO
 probably BaseConfig will be moved to Rule to allow for convenient
 map size and state count changes on the fly ('set' subcommand ?)
 * TODO
 displaying current config in imgui VS create some showconfig subcommand
 * TODO
 maybe config should be placed in Rules through composition?
 */

struct Rule1D : public Rule {
	Rule1D(Vec2<std::int32_t> map_resolution,
				 std::int32_t state_count,
				 std::shared_ptr<RuleConfig> rule_config)
			: Rule(map_resolution, state_count, std::move(rule_config)) {
	}

	void advanceReadRow() noexcept {
		base_config_.iteration = (base_config_.iteration + 1) % base_config_.map_resolution.y;
		glNamedBufferSubData(base_config_ubo_id_, offsetof(BaseConfig, iteration), sizeof(std::int32_t), &base_config_.iteration);
	}

	void step(const CellMap& cell_map) noexcept override {
		this->rule_config_->getShader()->bind();
		glBindBufferBase(
				GL_SHADER_STORAGE_BUFFER,
				STATE_MAP_SSBO_BINDING_LOCATION,
				cell_map.state_map_ssbo_id_
		);

		glDispatchCompute(static_cast<GLuint>(base_config_.map_resolution.x), 1, 1);
		advanceReadRow();

		this->rule_config_->getShader()->unbind();
	}

	void destroy() override {
		Rule::destroy();
	}
};

struct RuleConfig1DTotalistic : public RuleConfig {
	static constexpr Vec2<std::uint32_t> RANGE_LIM { 0, 10 };
	static constexpr std::size_t MAX_OPTIONS { 2 * RANGE_LIM.y + 1 };

	struct Config {
		std::int32_t range;
		alignas(sizeof(std::int32_t)) bool center_active;
		alignas(sizeof(Vec4<std::int32_t>)) std::int32_t survival_conditions_hashmap[MAX_OPTIONS]; // NOLINT interfacing with gl
		alignas(sizeof(Vec4<std::int32_t>)) std::int32_t birth_conditions_hashmap[MAX_OPTIONS];    // NOLINT interfacing with gl
	};

	Config config_;

	RuleConfig1DTotalistic( // NOLINT config is populated in the body
		std::int32_t range,
		bool center_active,
		std::span<const std::size_t> survival_conditions,
		std::span<const std::size_t> birth_conditions,
		std::shared_ptr<Shader> step_shader)
	: RuleConfig(std::move(step_shader)) {

		config_.range = range;
		config_.center_active = center_active;

		auto survival_conditions_hashmap = std::span(config_.survival_conditions_hashmap);
		auto birth_conditions_hashmap = std::span(config_.birth_conditions_hashmap);

		std::fill(survival_conditions_hashmap.begin(), survival_conditions_hashmap.end(), 0); // NOLINT 0/1
		std::fill(birth_conditions_hashmap.begin(), birth_conditions_hashmap.end(), 0); // NOLINT 0/1

		const auto setValues=
				[](std::span<std::int32_t> dst, std::span<const std::size_t> src) {
					for(const auto& value : src) {
						if(value < dst.size()) {
							dst[value] = 1;
						}
					}
				};
		setValues(survival_conditions_hashmap, survival_conditions);
		setValues(birth_conditions_hashmap, birth_conditions);
	}

	[[nodiscard]] std::size_t size() const override { return sizeof(Config); }
	[[nodiscard]] const void* data() const override { return &config_;       }

	void destroy() override {
		RuleConfig::destroy();
	}
};

struct RuleConfig1DBinary : public RuleConfig {
	static constexpr Vec2<std::uint32_t> RANGE_LIM{ 0, 4 };
	static constexpr std::uint32_t MAX_PATTERN_COUNT{ 16 }; // (2^(2 * RANGE_LIM.y + 1))/32

	struct Config {
		std::int32_t range;
		std::uint32_t pattern_match_code[MAX_PATTERN_COUNT]; // NOLINT interfacing with gl
	};

	Config config_;

	RuleConfig1DBinary(std::int32_t range, std::string_view pattern_match_code, // NOLINT init inside the body
										 std::shared_ptr<Shader> step_shader)
	: RuleConfig(std::move(step_shader)) {
		config_.range = range;
		parsePatternMatchCode(pattern_match_code);
	}

	void parsePatternMatchCode(std::string_view pattern_match_code) {
		auto wrapped_pattern_match_code = std::span(config_.pattern_match_code);
		std::fill(wrapped_pattern_match_code.begin(), wrapped_pattern_match_code.end(), 0);

		std::uint32_t i{ 0ul };
		constexpr std::uint32_t BIT_SHIFT{ 5ul }; // log2(sizeof(int))
		constexpr std::uint32_t MASK{ 0x1Ful };   // % 32
		for(const auto c : pattern_match_code) {
			if(c == '1') {
				wrapped_pattern_match_code[i >> BIT_SHIFT] |= (i << (i & MASK));
			}
		}
	}

	[[nodiscard]] std::size_t size() const override { return sizeof(Config); }
	[[nodiscard]] const void* data() const override { return &config_;       }

	void destroy() override {
		RuleConfig::destroy();
	}
};

} // namespace CA
namespace CLI {
// checks size of the vector in range [min_size, max_size]
struct VecSizeValidator : public CLI::Validator {
	explicit VecSizeValidator(std::size_t min_size, std::size_t max_size) {
		name_ = "VECSIZE";
		func_ = [min_size, max_size](const std::string& vec) {
			auto size = static_cast<std::size_t>(std::count(vec.begin(), vec.end(), ' '));
			if(vec.back() == ' ' || (size == 0 && !vec.empty())) {
				++size;
			}
			if (size < min_size || size > max_size) {
				return fmt::format("vector size {} isn't contained in {}, {}",
													 size, min_size, max_size);
			} else {
				return std::string();
			}
		};
	}
};
} // namespace CLI

int main(int argc, const char* const* argv) {
// parse command arguments //
	CLI::App app_cli_context{"CellSim cli based prototype"};

	std::size_t arg_map_width{ 0 };
	std::size_t arg_map_height{ 0 };
	std::vector<std::uint32_t> arg_colors;
	arg_colors.reserve(MAX_COLORS);

	app_cli_context.add_option(
		"--width", arg_map_width,
		"width of the cell map")
			->check(CLI::TypeValidator<std::size_t>());

	app_cli_context.add_option(
		"--height", arg_map_height,
		"height of the cell map")
			->check(CLI::TypeValidator<std::size_t>());

	app_cli_context.add_option(
		"--colors", arg_colors,
		"add history colors to cells in array of hex values (max 255)")
			->check(CLI::VecSizeValidator(1, 255));// NOLINT

	// 1D totalistic
	struct { // NOLINT initialized through ref
		std::uint32_t range;
		bool center_active;
		std::vector<std::size_t> survival_conditions;
		std::vector<std::size_t> birth_conditions;
	} cli_1dtotalistic_subcmd_data;
	auto* ca_type_1dtotalistic_subcmd = app_cli_context.add_subcommand(
			"1dtotalistic", "1d totalistic is one dimensional ca rule");
	ca_type_1dtotalistic_subcmd->add_option(
		"-r,--range", cli_1dtotalistic_subcmd_data.range,
		"describes range of the neighbourhood (0 ..<= 10)")
			->check(
					CLI::Range(CA::RuleConfig1DTotalistic::RANGE_LIM.x,
										         CA::RuleConfig1DTotalistic::RANGE_LIM.y));

	ca_type_1dtotalistic_subcmd->add_flag(
		"-e,--exclude-center",
		cli_1dtotalistic_subcmd_data.center_active,
		"if set then center cell won't be taken into account");

	ca_type_1dtotalistic_subcmd->add_option(
		"-s,--survive-conditions",
		cli_1dtotalistic_subcmd_data.survival_conditions,
		"array of sums which qualify cell for survival");

	ca_type_1dtotalistic_subcmd->add_option(
		"-b,--birth-conditions",
		cli_1dtotalistic_subcmd_data.birth_conditions,
		"array of sums which qualify cell for birth");

	// 1D binary
	struct { // NOLINT initialized through ref
		std::uint32_t range;
		std::string pattern_match_code;
	}cli_1dbinary_subcmd_data;
	auto* ca_type_1dbinary_subcmd = app_cli_context.add_subcommand(
			"1dbinary", "1d binary is one dimensional ca rule");
	ca_type_1dbinary_subcmd->add_option(
		"-r,--range",
		cli_1dbinary_subcmd_data.range,
		"describes range of the neighbourhood (0 ..<= 4)")
			->check(CLI::Range(CA::RuleConfig1DBinary::RANGE_LIM.x,
												 				CA::RuleConfig1DBinary::RANGE_LIM.y));

	ca_type_1dbinary_subcmd->add_option(
		"-p,--patern-match-code",
		cli_1dbinary_subcmd_data.pattern_match_code,
		"defines which patterns of size 2*<range>+1 qualify cell for birth/survival, 1 = qualifed, 0 = unqualified, goes from min to max");

	// parsing
	try { app_cli_context.parse(argc, argv); }
	catch(const CLI::ParseError& e) {
	  if(app_cli_context.exit(e) != static_cast<int>(CLI::ExitCodes::Success)) {
	    spdlog::error("[CLI11] {}: {}", e.get_name(), e.what());
	    return e.get_exit_code();
	  } else if(e.get_name() == "CallForHelp" 		||
							e.get_name() == "CallForAllHelp"  ||
							e.get_name() == "CallForVersion") {
			return e.get_exit_code();
		}
	}

	// zero state is always black
	constexpr std::uint32_t BLACK_NON_TRANSPARENT_HEX{ 0xFF };
	arg_colors.insert(arg_colors.begin(), BLACK_NON_TRANSPARENT_HEX);

	std::vector<Vec4<float>> colors(arg_colors.size());
	std::transform(
		arg_colors.cbegin(),arg_colors.cend(),
		colors.begin(),
		[](std::uint32_t color){
			constexpr std::uint32_t MASK{ 0xFF };
			constexpr float DIVIDER{ 255.f };
			return Vec4<float>{
			  static_cast<float>((color >> 24u))			  /DIVIDER, // NOLINT
				static_cast<float>((color >> 16u) & MASK) /DIVIDER, // NOLINT
				static_cast<float>((color >>  8u) & MASK) /DIVIDER, // NOLINT
				static_cast<float>((color >>  0u) & MASK) /DIVIDER  // NOLINT
			};
		}
	);
/////////////////////////////

// init window and gl //
	// test glfw initialization
  if(glfwInit() != GLFW_TRUE) {
    spdlog::critical("glfw init failed");
    return 1;
	}
  
  // test window creation
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, GLVERSION_MAJOR); 
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, GLVERSION_MINOR); 
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  constexpr int width = 640;
  constexpr int height = 480;
  auto* window = glfwCreateWindow(width, height, "CellSim", nullptr, nullptr); 
  if(window == nullptr) {
		spdlog::critical("window creation failed");
    return 1;
	}  
	
	glfwMakeContextCurrent(window);
  glfwSetErrorCallback([](int, const char* message){ spdlog::error("[glfw] {}", message); });

  if(gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)) == 0) { // NOLINT 
    spdlog::critical("glad loader failed");
    return 1;
	}

  glEnable(GL_DEBUG_OUTPUT);
  glDebugMessageCallback(glDebugCallback, nullptr);
////////////////////////
  try {

		std::unique_ptr<CA::Rule> rule{nullptr};
		std::shared_ptr<CA::RuleConfig> rule_config{nullptr};
		if (ca_type_1dtotalistic_subcmd->parsed()) {
			rule_config = std::make_shared<CA::RuleConfig1DTotalistic>(
					static_cast<std::int32_t>(cli_1dtotalistic_subcmd_data.range),
					cli_1dtotalistic_subcmd_data.center_active,
					cli_1dtotalistic_subcmd_data.survival_conditions,
					cli_1dtotalistic_subcmd_data.birth_conditions,
					std::make_shared<CA::CShader>("shaders/bin/1D_totalistic/comp.spv"));
			rule = std::make_unique<CA::Rule1D>(
					Vec2<std::int32_t>{static_cast<std::int32_t>(arg_map_width),
														 static_cast<std::int32_t>(arg_map_height)},
					static_cast<std::int32_t>(arg_colors.size()), rule_config);
		} else if (ca_type_1dbinary_subcmd->parsed()) {
			rule_config = std::make_shared<CA::RuleConfig1DBinary>(
					static_cast<std::int32_t>(cli_1dbinary_subcmd_data.range),
					cli_1dbinary_subcmd_data.pattern_match_code,
					std::make_shared<CA::CShader>("shaders/bin/1D_binary/comp.spv"));
			rule = std::make_unique<CA::Rule1D>(
					Vec2<std::int32_t>{static_cast<std::int32_t>(arg_map_width),
														 static_cast<std::int32_t>(arg_map_height)},
					static_cast<std::int32_t>(arg_colors.size()), rule_config);
		} else {
			throw std::runtime_error("passed subcommand not supported");
		}



  std::shared_ptr<CA::Shader> render_shader = std::make_shared<CA::VFShader>(
		VSH_RENDER_SHADER_PATH, FSH_RENDER_SHADER_PATH
	);
  CA::Renderer renderer(render_shader);
	renderer.setColors(colors);

	glfwSetWindowUserPointer(window, &renderer);
	glfwSetKeyCallback(window,
	[](GLFWwindow* win, int key, int, int action, int){
		auto* renderer_ptr = static_cast<CA::Renderer*>(glfwGetWindowUserPointer(win));
		if(action == GLFW_PRESS) {
			switch(key) {
			// zoom in, zoom out keys
			case GLFW_KEY_N: renderer_ptr->updateView({0.f, 0.f},-1.f); break;
			case GLFW_KEY_M: renderer_ptr->updateView({0.f, 0.f}, 1.f); break;
			// move keys
			case GLFW_KEY_W: renderer_ptr->updateView({ 0.f,-1.f}, 0.f); break;
			case GLFW_KEY_S: renderer_ptr->updateView({ 0.f, 1.f}, 0.f); break;
			case GLFW_KEY_A: renderer_ptr->updateView({ 1.f, 0.f}, 0.f); break;
			case GLFW_KEY_D: renderer_ptr->updateView({-1.f, 0.f}, 0.f); break;
			default: break;
		  }
		}
	});

	CA::CellMap cell_map(render_shader,arg_map_width,arg_map_height,
						 static_cast<std::int32_t>(colors.size()));

	cell_map.seed(0, 0); // NOLINT

  glClearColor(.0f, .0f, .0f, 1.f); // NOLINT
	auto last_frame_time{ static_cast<float>(glfwGetTime()) };
  while(glfwWindowShouldClose(window) == 0) {
    glfwPollEvents();
    glClear(GL_COLOR_BUFFER_BIT);

		auto frame_time = static_cast<float>(glfwGetTime());
		renderer.time_step_ = last_frame_time - frame_time;
		last_frame_time = frame_time;

    renderer.draw(cell_map);
		//rule->step(cell_map);

    glfwSwapBuffers(window);
  }

	glfwDestroyWindow(window);

	renderer.destroy();
	rule->destroy();
	cell_map.destroy();

	glfwTerminate();

  } catch(const std::exception& e) {
    spdlog::critical("{}", e.what());

    glfwDestroyWindow(window);   
    glfwTerminate();

    return 1;
  }

  return 0;
}