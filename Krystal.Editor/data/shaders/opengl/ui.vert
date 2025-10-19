#version 420 core

layout (location = 0) in vec2 v_Position;
layout (location = 1) in vec2 v_TextureCoords;
layout (location = 2) in vec4 v_Color;

out vec2 TextureCoords;
out vec4 VertexColor;

layout (std140, binding = 0) uniform Matrices
{
  mat4 view;
  mat4 projection;
  mat4 screenOrthoProjection;
};

void main()
{
  gl_Position = screenOrthoProjection * vec4(v_Position, 0.0, 1.0);
  TextureCoords = v_TextureCoords;
  VertexColor = v_Color;
}  