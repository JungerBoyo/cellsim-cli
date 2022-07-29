BIN_DIR := bin
SRC_DIR := src

GLSL_OPT := spirv-opt
GLSL_OPT_FLAGS := -O
GLSL := glslangValidator
GLSL_FLAGS := -G -V

all: $(BIN_DIR)/1D_binary/comp.spv $(BIN_DIR)/1D_totalistic/comp.spv $(BIN_DIR)/render_shader/vert.spv $(BIN_DIR)/render_shader/frag.spv

$(BIN_DIR)/1D_binary/comp.spv: $(SRC_DIR)/1D_binary/shader.comp $(BIN_DIR)/1D_binary
	$(GLSL) $(GLSL_FLAGS) -S comp $< -o $@
	$(GLSL_OPT) $(GLSL_OPT_FLAGS) $@ -o $@
$(BIN_DIR)/1D_totalistic/comp.spv: $(SRC_DIR)/1D_totalistic/shader.comp $(BIN_DIR)/1D_totalistic
	$(GLSL) $(GLSL_FLAGS) -S comp $< -o $@
	$(GLSL_OPT) $(GLSL_OPT_FLAGS) $@ -o $@
$(BIN_DIR)/render_shader/vert.spv:  $(SRC_DIR)/render_shader/shader.vert $(BIN_DIR)/render_shader
	$(GLSL) $(GLSL_FLAGS) $< -o $@
	$(GLSL_OPT) $(GLSL_OPT_FLAGS) $@ -o $@
$(BIN_DIR)/render_shader/frag.spv: $(SRC_DIR)/render_shader/shader.frag $(BIN_DIR)/render_shader
	$(GLSL) $(GLSL_FLAGS) $< -o $@
	$(GLSL_OPT) $(GLSL_OPT_FLAGS) $@ -o $@

$(SRC_DIR)/1D_binary/shader.comp:
$(SRC_DIR)/1D_totalistic/shader.comp:
$(SRC_DIR)/render_shader/shader.vert:
$(SRC_DIR)/render_shader/shader.frag:

$(BIN_DIR)/1D_binary $(BIN_DIR)/1D_totalistic $(BIN_DIR)/render_shader:
	mkdir $@ 

