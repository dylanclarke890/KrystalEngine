#version 330 core

uniform sampler2D atlas;

// Required for range-aware smoothing:
uniform float u_PixelRange = 4.0;   // same value you used during generation (e.g. 2.0)
uniform vec2  u_AtlasSize = vec2(377.0, 377.0);    // atlas width/height in pixels

in vec2 TexCoords;
out vec4 FragColor;

uniform vec3 textColor;

float median(float r, float g, float b) {
  return max(min(r, g), min(max(r, g), b));
}

void main() {
  vec3 msd = texture(atlas, TexCoords).rgb;

  // Signed distance in *normalized* MSDF units: 0 at edge, +/-0.5 far away.
  float sd = median(msd.r, msd.g, msd.b) - 0.5;

  // Convert the atlas pixel range to *screen* pixels at this exact scale.
  // A = atlas-pixels per screen-pixel (anisotropic-safe via derivatives).
  float Ax = length(vec2(dFdx(TexCoords.x), dFdy(TexCoords.x))) * u_AtlasSize.x;
  float Ay = length(vec2(dFdx(TexCoords.y), dFdy(TexCoords.y))) * u_AtlasSize.y;
  float A  = max(Ax, Ay);                 // worst-case axis

  // Distance in *screen pixels*:
  float distScreen = sd * (u_PixelRange / max(A, 1e-6));

  // 1 screen-pixel wide transition around the edge:
  float alpha = smoothstep(-0.5, 0.5, distScreen);

  // Optional: tiny gamma lift to reduce dark halos on light text over dark bg.
  // alpha = pow(alpha, 1.0/1.2);

  FragColor = vec4(textColor, alpha);
}
