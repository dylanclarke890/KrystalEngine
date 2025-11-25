#version 330

in vec2 TextureCoord;
in vec4 Colour;

out vec4 FragmentColour;

uniform sampler2D u_Texture;

void main()
{
	FragmentColour = Colour * texture(u_Texture, TextureCoord);
}