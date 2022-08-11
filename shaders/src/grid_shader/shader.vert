#version 450 core

layout(location = 0) in vec2 in_position;

layout(std140, binding = 0) uniform ViewConfig {
  vec2 offset;
  float scale;
  float aspect_ratio;
  vec4 out_line_color;
};

layout(std430, binding = 3) readonly buffer InstanceOffsets {
  vec2 instance_offsets[];
};

void main() {
  vec2 position = scale * (in_position + instance_offsets[gl_InstanceIndex]) + offset;
  gl_Position = vec4(aspect_ratio * position.x, position.y, 0.0, 1.0);
}
