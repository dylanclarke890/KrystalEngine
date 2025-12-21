#version 330

in vec2 TextureCoord;
in vec4 Colour;

out vec4 FragmentColour;

uniform sampler2D u_Texture;

void main()
{
  vec2 uv = TextureCoord;
  uv.y = 1.0 - uv.y; // Flip Y coordinate for texture
  FragmentColour = Colour * texture(u_Texture, uv);
}