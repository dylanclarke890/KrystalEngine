#version 420 core

layout (location = 0) in vec2 v_Position;
layout (location = 1) in vec2 v_TextureCoords;
layout (location = 2) in vec4 i_BackgroundColour;
layout (location = 3) in vec4 i_BorderColour;
layout (location = 4) in vec4 i_PositionAndSize;
layout (location = 5) in vec2 i_BorderThicknessAndRadius;

out vec2 TextureCoords;
out vec4 BackgroundColour;
out vec4 BorderColour;
out vec2 Size;
out vec2 BorderThicknessAndRadius;

uniform mat4 u_Projection;

void main()
{
  TextureCoords = v_TextureCoords;
  BackgroundColour = i_BackgroundColour;
  BorderColour = i_BorderColour;
  Size = i_PositionAndSize.zw;
  BorderThicknessAndRadius = i_BorderThicknessAndRadius;
  vec2 worldPos = i_PositionAndSize.xy + v_Position * i_PositionAndSize.zw;
  gl_Position = u_Projection * vec4(worldPos, 0.0, 1.0);
}  