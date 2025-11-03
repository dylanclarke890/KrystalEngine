#version 450 core

in vec2 TextureCoords;

out vec4 FragmentColour;

uniform sampler2D u_Texture;
uniform float u_Opacity;

void main()
{
  vec4 colour = texture(u_Texture, TextureCoords);
  FragmentColour = vec4(colour.rgb, colour.a * u_Opacity);
}