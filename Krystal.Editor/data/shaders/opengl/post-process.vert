#version 330

in vec2 v_Position;
in vec4 v_Colour0;
in vec2 v_TextureCoord0;

out vec4 Colour;
out vec2 TextureCoord;

void main()
{
  gl_Position = vec4(v_Position, 0.0, 1.0);
  Colour = v_Colour0;
  TextureCoord = v_TextureCoord0;
}
