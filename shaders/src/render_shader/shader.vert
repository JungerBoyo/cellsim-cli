#version 450 core 

layout(location = 0) in vec2 in_position;

layout(std140, binding = 0) uniform ViewConfig {
  vec2 offset;
  float scale;
};

const int MAX_COLORS = 256;

layout(std140, binding = 1) uniform Colors {
  vec4 colors[MAX_COLORS];
};

layout(std430, binding = 2) readonly buffer StateMap {
  int state_map[];
};

layout(std430, binding = 3) readonly buffer InstanceOffsets {
  vec2 instance_offsets[];
};

layout(location = 0) out vec4 out_color;

void main() {
  out_color = colors[state_map[gl_InstanceIndex]];
  gl_Position = vec4(scale * (in_position + (instance_offsets[gl_InstanceIndex] + offset)), 0.0, 1.0);
}
