#version 330

in vec2 TextureCoord;

out vec4 FragmentColour;

uniform sampler2D u_Texture;

void main()
{
  FragmentColour = texture(u_Texture, TextureCoord);
}