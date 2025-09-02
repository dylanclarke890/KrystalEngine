#version 330 core
layout (location = 0) in vec3 aPos;

out vec3 TexCoords;

uniform mat4 projection;
uniform mat4 view;

void main()
{
  TexCoords = aPos;
  mat4 rotView = mat4(mat3(view)); // remove translation from the view matrix
  vec4 position = rotView * projection * vec4(aPos, 1.0);
  gl_Position = position.xyww; // set w component to the z component to ensure depth is 1.0 (max depth).
}  