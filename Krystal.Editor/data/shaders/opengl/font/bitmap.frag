#version 330 core
in vec2 TexCoords;
out vec4 color;

uniform sampler2D atlas;

uniform vec3 textColor;

void main()
{
  float alpha = texture(atlas, TexCoords).r;
  color = vec4(textColor, alpha);
} 