#version 330

in vec2 TextureCoord;
in vec4 Colour;

out vec4 FragmentColour;

void main()
{
  FragmentColour = Colour;
}