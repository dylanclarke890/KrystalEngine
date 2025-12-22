#version 330 core

layout (location = 0) in vec2 v_Position;
layout (location = 1) in vec2 v_TextureCoords;

out vec2 TextureCoords;

uniform mat4 u_Transform;

void main()
{
  gl_Position = u_Transform * vec4(v_Position, 0.0, 1.0);
  TextureCoords = v_TextureCoords;
}  