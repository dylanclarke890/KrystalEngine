#version 330 core

in vec2 TexCoords;

uniform float u_PixelRange;
uniform vec2  u_AtlasSize;

uniform float u_OutlineWidth = 0.0; // in screen pixels
uniform vec3  u_OutlineColor = vec3(0.0);

uniform sampler2D atlas;
uniform vec3 u_TextColor;

// tweakable; ~1/24..1/16 works well depending on atlas quality
uniform float u_ErrorThreshold = 1.0/24.0;

out vec4 FragColor;

float median(vec3 v) {
  return max(min(v.r, v.g), min(max(v.r, v.g), v.b));
}

float atlasPixelsPerScreenPixel(vec2 uv, vec2 atlasSize) {
  // how many atlas texels map to 1 screen pixel (worst axis)
  vec2 dx = dFdx(uv) * atlasSize;
  vec2 dy = dFdy(uv) * atlasSize;
  return max(length(dx), length(dy));
}

void main() {
  vec4 s = texture(atlas, TexCoords);
  float sd_msdf = median(s.rgb) - 0.5;

  // Use true-distance channel near problematic regions (reduces color fringing).
  float tri = max(max(abs(s.r - s.g), abs(s.g - s.b)), abs(s.b - s.r));
  float sd = (tri < u_ErrorThreshold) ? (s.a - 0.5) : sd_msdf;

  float A = max(atlasPixelsPerScreenPixel(TexCoords, u_AtlasSize), 1e-6);
  float distScreen = sd * (u_PixelRange / A);

  float alpha = smoothstep(-0.5, 0.5, distScreen);
  FragColor = vec4(u_TextColor, alpha);
}
