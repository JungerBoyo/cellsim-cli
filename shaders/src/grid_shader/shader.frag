#version 450 core

precision mediump float;

layout(std140, binding = 0) uniform ViewConfig {
  vec2 offset;
  float scale;
  float aspect_ratio;
  vec4 out_line_color;
};

layout(location = 0) out vec4 out_fragment;

void main() {
  out_fragment = out_line_color;
}
