#version 420 core

in vec4 VertexColor;
in vec2 TextureCoords;

struct UIUboData {
  vec2 RectSize;
  vec2 BorderThicknessRadius; // x: border thickness, y: border radius
  vec4 FillColour;
};

layout (std140, binding = 5) uniform UIElementData
{
  UIUboData[1000] UboData;
};

out vec4 FragmentColor;

void main()
{
  FragmentColor = vec4(VertexColor);
}