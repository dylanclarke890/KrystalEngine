#version 330 core

in vec2 TexCoords;

uniform sampler2D atlas;
uniform vec3 u_TextColor;

out vec4 color;

void main() {
  float alpha = texture(atlas, TexCoords).r;
  color = vec4(u_TextColor, alpha);
} 