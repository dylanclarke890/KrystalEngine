#version 330 core

in vec2 TexCoords;

uniform float u_PixelRange;
uniform vec2  u_AtlasSize;

uniform sampler2D atlas;
uniform vec3 u_TextColor;

out vec4 FragColor;

float atlasPixelsPerScreenPixel(vec2 uv, vec2 atlasSize) {
  // how many atlas texels map to 1 screen pixel (worst axis)
  vec2 dx = dFdx(uv) * atlasSize;
  vec2 dy = dFdy(uv) * atlasSize;
  return max(length(dx), length(dy));
}

void main() {
  float s = texture(atlas, TexCoords).r;   // 0..1, 0.5 at the edge
  float sd = s - 0.5;                      // signed distance, normalized

  float A = max(atlasPixelsPerScreenPixel(TexCoords, u_AtlasSize), 1e-6);
  float distScreen = sd * (u_PixelRange / A);   // distance in *screen pixels*

  float alpha = smoothstep(-0.5, 0.5, distScreen);
  FragColor = vec4(u_TextColor, alpha);
}