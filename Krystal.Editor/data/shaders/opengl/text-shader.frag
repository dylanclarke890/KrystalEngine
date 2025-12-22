#version 420 core

in vec2 TextureCoords;

uniform sampler2D u_Atlas;
uniform vec3 u_TextColor;

layout (std140, binding = 3) uniform TestBinding
{
  vec4 testColor;
};

out vec4 FragmentColor;

// Defines:
// FONT_BITMAP: regular bitmap font
// FONT_SDF: single-channel signed distance field font
// FONT_MSDF: multi-channel signed distance field font
// FONT_MTSDF: multi-channel multi-distance signed distance field font
// FEATURE_OUTLINE: enables outline rendering (only for SDF/MSDF/MTSDF)

#if defined(FONT_SDF) || defined(FONT_MSDF) || defined(FONT_MTSDF)

    uniform vec2 u_UnitRange = vec2(1.0, 1.0);

    #if defined(FEATURE_OUTLINE)
  
      uniform vec3  u_OutlineColor = vec3(0.0);
      uniform float u_OutlineWidthAbsolute;
      uniform float u_OutlineWidthRelative;
      uniform float u_Threshold;

      #if defined(FONT_MTSDF)

        uniform float u_FontRoundingFactor;
        uniform float u_OutlineRoundingFactor;

      #endif

    #endif

  float Median(vec3 v) {
    return max(min(v.r, v.g), min(max(v.r, v.g), v.b));
  }

  float ScreenPxRange() {
    vec2 screenTexelSize =  vec2(1.0) / fwidth(TextureCoords);
    return max(0.5 * dot(u_UnitRange, screenTexelSize), 1.0);
  }

#endif

#if defined(FONT_BITMAP)

  void main() {
    float alpha = texture(u_Atlas, TextureCoords).r;
    // Pre-multiplied alpha
    FragmentColor = vec4(u_TextColor * alpha, alpha);
  }

#else

  // SDF/MSDF/MTSDF rendering
  // Atlas contains distance field data in the range [0,1] with 0.5 being the glyph edge.
  void main() {
  #if defined(FONT_SDF)

    float sdf = texture(u_Atlas, TextureCoords).r;

    float dInner = sdf;
    float dOuter = sdf;

  #elif defined(FONT_MSDF)

    float msdf = Median(texture(u_Atlas, TextureCoords).rgb);

    float dInner = msdf;
    float dOuter = msdf;

  #elif defined(FONT_MTSDF)

    vec4 distances = texture(u_Atlas, TextureCoords);
    float msdf = Median(distances.rgb);
    float sdf = distances.a;
    msdf = min(msdf, sdf + 0.1); // avoid artifacts around corners

    #if defined(FEATURE_OUTLINE)

      // MTSDF ONLY: blend between sharp and rounded corners
      float dInner = mix(msdf, sdf, u_FontRoundingFactor);
      float dOuter = mix(msdf, sdf, u_OutlineRoundingFactor);

    #else

      float dInner = msdf;
      float dOuter = msdf;

    #endif

  #endif

  #if defined(FEATURE_OUTLINE)

    // typically 0.5 is the threshold, >0.5 inside <0.5 outside
    float invertedThreshold = 1.0 - u_Threshold; // because I want the ui to be +larger -smaller
    float width = ScreenPxRange();
    float inner = width * (dInner - invertedThreshold) + 0.5;
    float outer = width * (dOuter - invertedThreshold + u_OutlineWidthRelative) + 0.5 + u_OutlineWidthAbsolute;

    float innerOpacity = clamp(inner, 0.0, 1.0);
    float outerOpacity = clamp(outer, 0.0, 1.0);
    vec4 innerColor = vec4(u_TextColor, 1);
    vec4 outerColor = vec4(u_OutlineColor, 1);

    float ring = max(outerOpacity - innerOpacity, 0);
    vec4 color = innerColor * innerOpacity + outerColor * ring;
    FragmentColor = color;

  #else // no outline

    float width = ScreenPxRange();
    float alpha = clamp(width * (dInner - 0.5) + 0.5, 0.0, 1.0);
    FragmentColor = vec4(u_TextColor, alpha);
  
  #endif

  }
#endif
