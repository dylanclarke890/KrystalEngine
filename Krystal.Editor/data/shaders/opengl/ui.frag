#version 420 core

in vec2 TextureCoords;

uniform vec3 u_BackgroundColour;
out vec4 FragmentColor;

void main()
{
  FragmentColor = vec4(u_BackgroundColour, 1.0);
}