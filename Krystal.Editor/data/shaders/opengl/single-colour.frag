#version 330 core

out vec4 FragmentColour;

uniform vec4 u_Colour = vec4(0.04, 0.28, 0.26, 1.0);

void main()
{    
  FragmentColour = u_Colour;
}