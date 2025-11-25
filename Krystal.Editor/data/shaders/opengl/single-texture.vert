#version 420 core

layout (location = 0) in vec2 v_Position;
layout (location = 1) in vec2 v_TextureCoords;
layout (location = 2) in vec4 i_PositionAndSize;
layout (location = 3) in vec4 i_BackgroundColour;

out vec2 TextureCoords;
out vec4 BackgroundColour;
out vec2 Size;

uniform mat4 u_Projection;

void main()
{
  vec2 worldPos = i_PositionAndSize.xy + v_Position * i_PositionAndSize.zw;
  gl_Position = u_Projection * vec4(worldPos, 0.0, 1.0);
  TextureCoords = v_TextureCoords;
  BackgroundColour = i_BackgroundColour;
  Size = i_PositionAndSize.zw;
}  