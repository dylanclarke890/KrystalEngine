#version 330

in vec2 v_Position;
in vec4 v_Colour0;
in vec2 v_TextureCoord0;

out vec4 Colour;
out vec2 TextureCoord;

uniform mat4 u_Transform;

void main()
{
  gl_Position = u_Transform * vec4(v_Position, 0.0, 1.0);
	Colour = v_Colour0;
	TextureCoord = v_TextureCoord0;
}