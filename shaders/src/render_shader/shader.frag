#version 450 core 

precision mediump float;

layout(location = 0) in vec4 in_color;

layout(location = 1) out vec4 out_fragment;

void main() {
  out_fragment = in_color;  
}