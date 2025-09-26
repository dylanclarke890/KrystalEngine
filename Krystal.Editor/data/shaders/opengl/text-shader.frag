#version 330 core

in vec2 TextureCoords;

uniform sampler2D u_Atlas;
uniform vec3 u_TextColor;

out vec4 FragmentColor;

#if defined(FONT_SDF) || defined(FONT_MSDF) || defined(FONT_MTSDF)

  uniform float u_PixelRange;
  uniform vec2  u_AtlasSize;
  uniform float u_ErrorThreshold = 1.0/24.0;

  float Median(vec3 v) {
    return max(min(v.r, v.g), min(max(v.r, v.g), v.b));
  }

  float AtlasPixelsPerScreenPixel(vec2 uv, vec2 atlasSize) {
    // how many atlas texels map to 1 screen pixel (worst axis)
    vec2 dx = dFdx(uv) * atlasSize;
    vec2 dy = dFdy(uv) * atlasSize;
    return max(length(dx), length(dy));
  }

#endif

#if defined(FONT_BITMAP)

  void main() {
    float alpha = texture(u_Atlas, TextureCoords).r;
    FragmentColor = vec4(u_TextColor, alpha);
  }

#else
  void main() {
  #if defined(FONT_SDF)

    float sample = texture(u_Atlas, TextureCoords).r;   // 0..1, 0.5 at the edge
    float signedDistance = sample - 0.5;                // signed distance, normalized
    float A = max(AtlasPixelsPerScreenPixel(TextureCoords, u_AtlasSize), 1e-6);

  #elif defined(FONT_MSDF)

    vec3 sample = texture(u_Atlas, TextureCoords).rgb;
    float signedDistance = Median(sample) - 0.5;
    float A = max(AtlasPixelsPerScreenPixel(TextureCoords, u_AtlasSize), 1e-6);

  #elif defined(FONT_MTSDF)

    vec4 sample = texture(u_Atlas, TextureCoords);
    float msdf = Median(sample.rgb) - 0.5;

    // Use true-distance channel near problematic regions (reduces color fringing).
    float tri = max(max(abs(sample.r - sample.g), abs(sample.g - sample.b)), abs(sample.b - sample.r));
    float signedDistance = (tri < u_ErrorThreshold) ? (sample.a - 0.5) : msdf;
    float A = max(AtlasPixelsPerScreenPixel(TextureCoords, u_AtlasSize), 1e-6);

  #endif

    float distanceInScreenPixels = signedDistance * (u_PixelRange / A);
    float alpha = smoothstep(-0.5, 0.5, distanceInScreenPixels);

    FragmentColor = vec4(u_TextColor, alpha);
  }
#endif
