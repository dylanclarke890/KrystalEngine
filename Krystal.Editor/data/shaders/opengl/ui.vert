#version 420 core

layout (location = 0) in vec2 v_Position;
layout (location = 1) in vec2 v_TextureCoords;
layout (location = 2) in vec4 i_PositionAndSize;
layout (location = 3) in vec4 i_BorderWidths;
layout (location = 4) in vec4 i_BackgroundColour;
layout (location = 5) in vec4 i_BorderColourLeft;
layout (location = 6) in vec4 i_BorderColourRight;
layout (location = 7) in vec4 i_BorderColourTop;
layout (location = 8) in vec4 i_BorderColourBottom;

out vec2 TextureCoords;

out vec2 Size;
out vec4 BorderWidths;

out vec4 BackgroundColour;
out vec4 BorderColourLeft;
out vec4 BorderColourRight;
out vec4 BorderColourTop;
out vec4 BorderColourBottom;

uniform mat4 u_Projection;

void main()
{
  TextureCoords = v_TextureCoords;

  vec2 worldPos = i_PositionAndSize.xy + v_Position * i_PositionAndSize.zw;
  gl_Position = u_Projection * vec4(worldPos, 0.0, 1.0);

  Size = i_PositionAndSize.zw;
  BorderWidths = i_BorderWidths;

  BackgroundColour = i_BackgroundColour;
  BorderColourLeft = i_BorderColourLeft;
  BorderColourRight = i_BorderColourRight;
  BorderColourTop = i_BorderColourTop;
  BorderColourBottom = i_BorderColourBottom;
}