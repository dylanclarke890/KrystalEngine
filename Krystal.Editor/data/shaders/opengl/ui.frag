#version 420 core

in vec2 TextureCoords;
in vec4 BackgroundColour;
in vec4 BorderColourLeft;
in vec4 BorderColourRight;
in vec4 BorderColourTop;
in vec4 BorderColourBottom;
in vec2 Size;
in vec4 BorderWidths;
in vec4 BorderRadiiTop; // TR, TL
in vec4 BorderRadiiBottom; // BR, BL

out vec4 FragmentColour;

void main()
{
  float rx = TextureCoords.x * Size.x;
  float ry = TextureCoords.y * Size.y;
  float left = BorderWidths.x;
  float right = Size.x - BorderWidths.y;
  float top = Size.y - BorderWidths.z;
  float bottom = BorderWidths.w;

  vec4 colour = BackgroundColour;
  // Left border
  if (rx < left)
    colour = BorderColourLeft;
  // Right border
  else if (rx > right)
    colour = BorderColourRight;
  // Top border
  else if (ry > top)
    colour = BorderColourTop;
  // Bottom border
  else if (ry < bottom)
    colour = BorderColourBottom;

  FragmentColour = colour;
}