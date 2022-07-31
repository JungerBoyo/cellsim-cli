#version 450 core 

precision mediump float;

layout(location = 0) out vec4 out_fragment;

layout(location = 2) flat in vec4 in_color;

void main() {
  out_fragment = in_color;  
}