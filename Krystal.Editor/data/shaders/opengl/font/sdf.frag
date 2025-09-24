#version 330 core
in vec2 TexCoords;
out vec4 color;

uniform sampler2D atlas;
uniform vec3 textColor;

void main()
{    
  float sd = texture(atlas, TexCoords).r - 0.5;
  float w = fwidth(sd);
  float alpha = smoothstep(-w, +w, sd);
  color = vec4(textColor.rgb, alpha);
}
