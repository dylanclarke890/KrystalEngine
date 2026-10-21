#include "Krystal.Gfx/Render2D/Renderer2D.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Log/ILogger.hpp"
#include "Krystal.Core/Utils/Move.hpp"
#include "Krystal.Core/Visitor.hpp"
#include "Krystal.Gfx/Render2D/Dashing.hpp"
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstring>

namespace krys::gfx
{
  namespace
  {
    // A quad drawn as a 4-vertex strip per instance; the corner comes from gl_VertexID. The rect is in local
    // coordinates, mapped to target pixels by the instance's transform. The quad is a target pixel larger
    // than the rect on every side, so the fragments in the antialiased band outside the edge run.
    constexpr const char8_t *RectVertexSource = u8R"(#version 450
layout(binding = 0, std140) uniform Frame
{
  vec2 u_ViewportSize;
  vec2 u_Origin; // the target pixel at the viewport's top-left
  float u_Dither; // how far gradients are dithered: a colour step, or 0
};

layout(location = 0) in vec4 a_Rect;        // x, y, width, height in local coordinates
layout(location = 1) in vec4 a_Radii;       // top-left, top-right, bottom-right, bottom-left
layout(location = 2) in vec4 a_Linear;      // the transform's a, b, c, d
layout(location = 3) in vec2 a_Translation; // the transform's e, f
layout(location = 4) in vec4 a_UV;          // texture coordinates at the top-left and bottom-right corners
layout(location = 5) in vec4 a_Colour;      // sRGB, straight alpha
layout(location = 6) in uint a_Flags;
layout(location = 7) in vec4 a_HoleRect;    // x, y, width, height in local coordinates
layout(location = 8) in vec4 a_HoleRadii;   // top-left, top-right, bottom-right, bottom-left

layout(location = 0) out vec2 v_Position; // relative to the rect's centre, in local coordinates
layout(location = 1) flat out vec2 v_HalfSize;
layout(location = 2) flat out vec4 v_Radii;
layout(location = 3) flat out mat2 v_InverseTranspose; // takes locations 3 and 4
layout(location = 5) flat out vec4 v_Colour;           // sRGB, premultiplied
layout(location = 6) out vec2 v_UV;
layout(location = 7) flat out vec4 v_UVRect;
layout(location = 8) flat out uint v_Flags;
layout(location = 9) flat out vec4 v_Hole; // the hole's centre relative to the rect's, and its half size
layout(location = 10) flat out vec4 v_HoleRadii;
layout(location = 11) out vec2 v_Pixel; // in target pixels

void main()
{
  mat2 linear = mat2(a_Linear.xy, a_Linear.zw);
  mat2 inverseLinear = inverse(linear);
  // How far one target pixel reaches along each local axis.
  vec2 pixelInLocal = vec2(length(vec2(inverseLinear[0][0], inverseLinear[1][0])),
                           length(vec2(inverseLinear[0][1], inverseLinear[1][1])));

  vec2 corner = vec2(gl_VertexID & 1, gl_VertexID >> 1);
  vec2 halfSize = a_Rect.zw * 0.5;
  // A shadow's quad reaches three standard deviations further, where its blur fades out.
  float blurReach = (a_Flags & 64u) != 0u ? 3.0 * a_UV.x : 0.0;
  vec2 position = (corner * 2.0 - 1.0) * (halfSize + pixelInLocal + blurReach);
  vec2 local = a_Rect.xy + halfSize + position;
  vec2 pixel = linear * local + a_Translation;
  vec2 ndc = (pixel - u_Origin) / u_ViewportSize * 2.0 - 1.0;
  gl_Position = vec4(ndc.x, -ndc.y, 0.0, 1.0);

  v_Position = position;
  v_HalfSize = halfSize;
  v_Radii = a_Radii;
  v_InverseTranspose = transpose(inverseLinear);
  v_UV = mix(a_UV.xy, a_UV.zw, (local - a_Rect.xy) / a_Rect.zw);
  v_UVRect = a_UV;
  v_Flags = a_Flags;
  v_Hole = vec4(a_HoleRect.xy + a_HoleRect.zw * 0.5 - (a_Rect.xy + halfSize), a_HoleRect.zw * 0.5);
  v_HoleRadii = a_HoleRadii;
  v_Pixel = pixel;
  v_Colour = vec4(a_Colour.rgb * a_Colour.a, a_Colour.a);
}
)";

    constexpr const char8_t *RectFragmentSource = u8R"(#version 450
layout(binding = 0, std140) uniform Frame
{
  vec2 u_ViewportSize;
  vec2 u_Origin; // the target pixel at the viewport's top-left
  float u_Dither; // how far gradients are dithered: a colour step, or 0
};

layout(binding = 0) uniform sampler2D u_Texture;

layout(location = 0) in vec2 v_Position;
layout(location = 1) flat in vec2 v_HalfSize;
layout(location = 2) flat in vec4 v_Radii;
layout(location = 3) flat in mat2 v_InverseTranspose;
layout(location = 5) flat in vec4 v_Colour;
layout(location = 6) in vec2 v_UV;
layout(location = 7) flat in vec4 v_UVRect;
layout(location = 8) flat in uint v_Flags;
layout(location = 9) flat in vec4 v_Hole;
layout(location = 10) flat in vec4 v_HoleRadii;
layout(location = 11) in vec2 v_Pixel;

struct Clip
{
  vec4 Rect;               // x, y, width, height in the clip's coordinates
  vec4 Radii;              // top-left, top-right, bottom-right, bottom-left
  vec4 InverseLinear;      // the inverse transform's a, b, c, d: from target pixels to the clip
  vec4 InverseTranslation; // the inverse transform's e, f; then 1 for a clip-out
};

layout(binding = 1, std140) uniform Clips
{
  uint u_ClipCount;
  uint u_ClipMaskMode;    // 0: none; 1: clip to the mask; 2: clip it out
  ivec4 u_ClipMaskBounds; // the target pixels the mask covers: x, y, width, height
  ivec4 u_ClipMaskOrigin; // the mask's first texel
  Clip u_Clips[16];
};

// The path clips' coverage, a texel per target pixel.
layout(binding = 1) uniform sampler2D u_ClipMask;

// What's beneath a draw with a blend mode, premultiplied, laid out as the pass.
layout(binding = 2) uniform sampler2D u_Backdrop;

// The batch's gradient: its stops are linear in premultiplied sRGB-encoded colours.
layout(binding = 2, std140) uniform Gradient
{
  uint u_GradientShape;  // 1: linear; 2: radial; 3: conic
  uint u_GradientSpread; // 0: pad; 1: reflect; 2: repeat
  uint u_GradientStopCount;
  vec4 u_GradientInverseLinear;      // from target pixels into the shape's space: a, b, c, d
  vec4 u_GradientInverseTranslation; // e, f
  vec4 u_GradientRadial;             // the second centre relative to the first, then the two radii
  vec4 u_GradientOffsets[64];        // four stops' offsets each
  vec4 u_GradientColours[256];
};

// The batch's pattern: its texture is one tile, repeating.
layout(binding = 3, std140) uniform Pattern
{
  vec4 u_PatternInverseLinear;      // from target pixels into pattern space: a, b, c, d
  vec4 u_PatternInverseTranslation; // e, f
  vec4 u_PatternTileAndSpacing;     // the tile's size, then the spacing after it
  uvec4 u_PatternFlags;             // 1: straight alpha; 2: an sRGB texture; 4, 8: clamp along x, y
};

layout(binding = 3) uniform sampler2D u_Pattern;

// The batch's colour matrix, a layer's filter: on straight-alpha colours, c' = matrix * c + offset.
layout(binding = 4, std140) uniform ColourMatrix
{
  mat4 u_ColourMatrix;
  vec4 u_ColourMatrixOffset;
};

layout(location = 0) out vec4 o_Colour;

const uint Textured = 1u;
const uint StraightAlpha = 2u;
const uint LinearTexels = 4u;
const uint Mask = 8u;
const uint Hole = 16u;
const uint GradientPaint = 32u;
const uint Shadow = 64u;
const uint AlphaMask = 128u;
const uint PatternPaint = 8192u;
const uint ColourMatrixFilter = 16384u;
const uint BlendModeShift = 8u;

const uint LinearShape = 1u;
const uint RadialShape = 2u;
const uint ConicShape = 3u;
const uint SpreadReflect = 1u;
const uint SpreadRepeat = 2u;

// BlendMode's values.
const uint BlendMultiply = 2u;
const uint BlendScreen = 3u;
const uint BlendDarken = 4u;
const uint BlendLighten = 5u;
const uint BlendOverlay = 6u;
const uint BlendColorDodge = 7u;
const uint BlendColorBurn = 8u;
const uint BlendHardLight = 9u;
const uint BlendSoftLight = 10u;
const uint BlendDifference = 11u;
const uint BlendExclusion = 12u;
const uint BlendHue = 13u;
const uint BlendSaturation = 14u;
const uint BlendColor = 15u;
const uint BlendLuminosity = 16u;

vec3 LinearToSrgb(vec3 c)
{
  return mix(c * 12.92, 1.055 * pow(c, vec3(1.0 / 2.4)) - 0.055, greaterThan(c, vec3(0.0031308)));
}

// How much of a pixel an edge covers, from the pixel's distance past the edge in local units and the
// edge's outward normal. The distance grows by length(inverseTranspose * normal) local units per target
// pixel, so that converts it to pixels; it's exactly 1 without a transform, which keeps pixel-aligned edges
// sharp. Coverage ramps over one pixel centred on the edge.
float EdgeCoverage(float distance, vec2 normal, mat2 inverseTranspose)
{
  return clamp(0.5 - distance / length(inverseTranspose * normal), 0.0, 1.0);
}

// How much of the pixel at p (relative to the box's centre, in local units) a rounded box covers. y points
// down, so the top corners are where p.y < 0.
float Coverage(vec2 p, vec2 halfSize, vec4 radii, mat2 inverseTranspose)
{
  vec2 side = vec2(p.x < 0.0 ? -1.0 : 1.0, p.y < 0.0 ? -1.0 : 1.0);
  float r = p.x < 0.0 ? (p.y < 0.0 ? radii.x : radii.w) : (p.y < 0.0 ? radii.y : radii.z);
  vec2 q = abs(p) - halfSize + r;
  if (r > 0.0 && q.x > 0.0 && q.y > 0.0)
  {
    // Around a rounded corner: the distance to its arc.
    return EdgeCoverage(length(q) - r, normalize(q) * side, inverseTranspose);
  }

  // Elsewhere the box's vertical and horizontal edges cover the pixel independently, so a square corner
  // halfway across a pixel and halfway down it covers a quarter of it, as its area does.
  vec2 d = abs(p) - halfSize;
  return EdgeCoverage(d.x, vec2(side.x, 0.0), inverseTranspose)
         * EdgeCoverage(d.y, vec2(0.0, side.y), inverseTranspose);
}

// How much of the pixel the clips let through: each shape clip's coverage, or one minus it for a clip-out,
// multiplied together, and then the path clips' mask's.
float ClipCoverage()
{
  float coverage = 1.0;
  for (uint i = 0u; i < u_ClipCount; ++i)
  {
    Clip clip = u_Clips[i];
    mat2 inverseLinear = mat2(clip.InverseLinear.xy, clip.InverseLinear.zw);
    vec2 local = inverseLinear * v_Pixel + clip.InverseTranslation.xy;
    vec2 halfSize = clip.Rect.zw * 0.5;
    float inside = Coverage(local - (clip.Rect.xy + halfSize), halfSize, clip.Radii, transpose(inverseLinear));
    coverage *= clip.InverseTranslation.z != 0.0 ? 1.0 - inside : inside;
  }

  if (u_ClipMaskMode != 0u)
  {
    // Outside its bounds, the mask covers nothing.
    ivec2 texel = ivec2(floor(v_Pixel)) - u_ClipMaskBounds.xy;
    bool inside = all(greaterThanEqual(texel, ivec2(0))) && all(lessThan(texel, u_ClipMaskBounds.zw));
    float mask = inside ? texelFetch(u_ClipMask, texel + u_ClipMaskOrigin.xy, 0).r : 0.0;
    coverage *= u_ClipMaskMode == 1u ? mask : 1.0 - mask;
  }
  return coverage;
}

// A texel, sRGB-encoded and premultiplied.
vec4 SampleTexture(vec2 dx, vec2 dy)
{
  // Stay half a texel inside the source rect, so filtering never reads the texels around it.
  vec2 halfTexel = 0.5 / vec2(textureSize(u_Texture, 0));
  vec2 low = min(v_UVRect.xy, v_UVRect.zw) + halfTexel;
  vec2 high = max(v_UVRect.xy, v_UVRect.zw) - halfTexel;
  vec2 uv = clamp(v_UV, min(low, high), max(low, high));
  vec4 texel = textureGrad(u_Texture, uv, dx, dy);
  if ((v_Flags & Mask) != 0u)
  {
    // Coverage, not a colour: nothing to premultiply or convert.
    return vec4(texel.r);
  }
  if ((v_Flags & AlphaMask) != 0u)
  {
    // A colour image's alpha as coverage: its silhouette.
    return vec4(texel.a);
  }

  // The GPU decoded each channel of an sRGB texture; encoding each back gives the stored values, whether
  // they're premultiplied or not.
  if ((v_Flags & LinearTexels) != 0u)
  {
    texel.rgb = LinearToSrgb(texel.rgb);
  }
  if ((v_Flags & StraightAlpha) != 0u)
  {
    texel.rgb *= texel.a;
  }
  return texel;
}

float GradientOffset(uint i)
{
  return u_GradientOffsets[i >> 2u][i & 3u];
}

// The gradient's colour at the pixel, premultiplied; transparent where a radial gradient has no circle.
// The pattern's premultiplied colour at the pixel. Without spacing along an axis, the texture is sampled
// continuously there, so the sampler's repeat wraps filtering across tile edges.
vec4 PatternColour()
{
  mat2 inverseLinear = mat2(u_PatternInverseLinear.xy, u_PatternInverseLinear.zw);
  vec2 p = inverseLinear * v_Pixel + u_PatternInverseTranslation.xy;
  vec2 tile = u_PatternTileAndSpacing.xy;
  vec2 spacing = u_PatternTileAndSpacing.zw;
  vec2 cellSize = tile + spacing;
  vec2 cell = p - floor(p / cellSize) * cellSize;
  if ((spacing.x > 0.0 && cell.x >= tile.x) || (spacing.y > 0.0 && cell.y >= tile.y))
  {
    return vec4(0.0);
  }
  vec2 uv = vec2(spacing.x > 0.0 ? cell.x : p.x, spacing.y > 0.0 ? cell.y : p.y) / tile;
  // A clamped axis stays half a texel inside the tile, as the sampler's clamp would keep it.
  vec2 halfTexel = 0.5 / vec2(textureSize(u_Pattern, 0));
  if ((u_PatternFlags.x & 4u) != 0u)
  {
    uv.x = clamp(uv.x, halfTexel.x, 1.0 - halfTexel.x);
  }
  if ((u_PatternFlags.x & 8u) != 0u)
  {
    uv.y = clamp(uv.y, halfTexel.y, 1.0 - halfTexel.y);
  }
  // Per target pixel, p moves by the inverse's columns.
  vec4 texel = textureGrad(u_Pattern, uv, inverseLinear[0] / tile, inverseLinear[1] / tile);
  if ((u_PatternFlags.x & 2u) != 0u)
  {
    texel.rgb = LinearToSrgb(texel.rgb);
  }
  if ((u_PatternFlags.x & 1u) != 0u)
  {
    texel.rgb *= texel.a;
  }
  return texel;
}

vec4 GradientColour()
{
  mat2 inverseLinear = mat2(u_GradientInverseLinear.xy, u_GradientInverseLinear.zw);
  vec2 p = inverseLinear * v_Pixel + u_GradientInverseTranslation.xy;

  float t = p.x;
  if (u_GradientShape == RadialShape)
  {
    // The largest t whose circle, with a radius that isn't negative, passes through p: the roots of
    // a t^2 - 2 b t + c = 0, taken as q / a and c / q so neither loses precision.
    vec2 centre = u_GradientRadial.xy;
    float r0 = u_GradientRadial.z;
    float dr = u_GradientRadial.w - r0;
    float a = dot(centre, centre) - dr * dr;
    float b = dot(p, centre) + r0 * dr;
    float c = dot(p, p) - r0 * r0;
    float discriminant = b * b - a * c;
    if (discriminant < 0.0)
    {
      return vec4(0.0);
    }
    float root = sqrt(discriminant);
    float q = b < 0.0 ? b - root : b + root;
    bool hasFirst = a != 0.0;
    bool hasSecond = q != 0.0;
    float first = hasFirst ? q / a : 0.0;
    float second = hasSecond ? c / q : 0.0;
    if (hasFirst && hasSecond && second > first)
    {
      float larger = second;
      second = first;
      first = larger;
    }
    else if (!hasFirst)
    {
      first = second;
      hasFirst = hasSecond;
      hasSecond = false;
    }
    if (hasFirst && r0 + first * dr >= 0.0)
    {
      t = first;
    }
    else if (hasSecond && r0 + second * dr >= 0.0)
    {
      t = second;
    }
    else
    {
      return vec4(0.0);
    }
  }
  else if (u_GradientShape == ConicShape)
  {
    // Clockwise from x, which y pointing down makes atan's direction.
    t = p == vec2(0.0) ? 0.0 : atan(p.y, p.x) / 6.28318530718;
    t = t < 0.0 ? t + 1.0 : t;
  }

  if (u_GradientSpread == SpreadRepeat)
  {
    t = fract(t);
  }
  else if (u_GradientSpread == SpreadReflect)
  {
    t = mod(t, 2.0);
    t = t > 1.0 ? 2.0 - t : t;
  }

  // The last stop at or before t, by binary search; past the ends, the end stops' colours carry on.
  uint count = u_GradientStopCount;
  uint low = 0u;
  uint high = count;
  while (low < high)
  {
    uint middle = (low + high) / 2u;
    if (GradientOffset(middle) <= t)
    {
      low = middle + 1u;
    }
    else
    {
      high = middle;
    }
  }
  if (low == 0u)
  {
    return u_GradientColours[0];
  }
  if (low == count)
  {
    return u_GradientColours[count - 1u];
  }
  float from = GradientOffset(low - 1u);
  return mix(u_GradientColours[low - 1u], u_GradientColours[low], (t - from) / (GradientOffset(low) - from));
}

// The error function, within 1.5e-7 (Abramowitz and Stegun 7.1.26).
float Erf(float x)
{
  float t = 1.0 / (1.0 + 0.3275911 * abs(x));
  float polynomial = ((((1.061405429 * t - 1.453152027) * t + 1.421413741) * t - 0.284496736) * t + 0.254829592) * t;
  return sign(x) * (1.0 - polynomial * exp(-x * x));
}

// How much of a Gaussian with standard deviation sigma, centred on p, falls between low and high.
float GaussianMass(float low, float high, float p, float sigma)
{
  float k = 0.70710678 / sigma;
  return 0.5 * (Erf((high - p) * k) - Erf((low - p) * k));
}

// A rounded box (centred on the origin, y down) blurred by a Gaussian, at p. A plain box is exact: the
// blur's mass over it in x times that in y. With rounded corners, rows within four standard deviations of p
// are taken in sixteen bands, each weighted by the Gaussian's mass over it and as wide as the box is
// halfway down it.
float BlurredBox(vec2 p, vec2 halfSize, vec4 radii, float sigma)
{
  if (radii == vec4(0.0))
  {
    return GaussianMass(-halfSize.x, halfSize.x, p.x, sigma) * GaussianMass(-halfSize.y, halfSize.y, p.y, sigma);
  }

  float low = max(-halfSize.y, p.y - 4.0 * sigma);
  float high = min(halfSize.y, p.y + 4.0 * sigma);
  if (low >= high)
  {
    return 0.0;
  }
  float band = (high - low) / 16.0;
  float k = 0.70710678 / sigma;
  float previous = Erf((low - p.y) * k);
  float total = 0.0;
  for (int i = 0; i < 16; ++i)
  {
    float y = low + (float(i) + 0.5) * band;
    float next = Erf((low + float(i + 1) * band - p.y) * k);
    float weight = 0.5 * (next - previous);
    previous = next;

    // How far the corners' arcs cut into the row: top-left, top-right, bottom-right, bottom-left.
    float left = y < 0.0 ? radii.x : radii.w;
    float right = y < 0.0 ? radii.y : radii.z;
    float fromEdge = halfSize.y - abs(y);
    float leftInset = fromEdge < left ? left - sqrt(max(left * left - (left - fromEdge) * (left - fromEdge), 0.0)) : 0.0;
    float rightInset = fromEdge < right ? right - sqrt(max(right * right - (right - fromEdge) * (right - fromEdge), 0.0)) : 0.0;
    total += weight * GaussianMass(-halfSize.x + leftInset, halfSize.x - rightInset, p.x, sigma);
  }
  return total;
}

// An 8-by-8 ordered dither threshold for the pixel, from 0 to 1: the bits of x ^ y and x interleaved and
// reversed.
float Bayer()
{
  uvec2 p = uvec2(ivec2(floor(v_Pixel))) & 7u;
  uint v = p.x ^ p.y;
  uint bits = ((v & 1u) << 5u) | ((p.x & 1u) << 4u) | ((v & 2u) << 2u) | ((p.x & 2u) << 1u)
              | ((v & 4u) >> 1u) | ((p.x & 4u) >> 2u);
  return (float(bits) + 0.5) / 64.0;
}

// The blend functions of Compositing and Blending Level 1, on straight colours: b is the backdrop's and s
// the source's.
vec3 HardLight(vec3 b, vec3 s)
{
  vec3 screen = 2.0 * s - 1.0;
  return mix(b + screen - b * screen, b * 2.0 * s, lessThanEqual(s, vec3(0.5)));
}

vec3 ColorDodge(vec3 b, vec3 s)
{
  vec3 result = min(vec3(1.0), b / max(1.0 - s, 1e-6));
  result = mix(result, vec3(1.0), greaterThanEqual(s, vec3(1.0)));
  return mix(result, vec3(0.0), lessThanEqual(b, vec3(0.0)));
}

vec3 ColorBurn(vec3 b, vec3 s)
{
  vec3 result = 1.0 - min(vec3(1.0), (1.0 - b) / max(s, 1e-6));
  result = mix(result, vec3(0.0), lessThanEqual(s, vec3(0.0)));
  return mix(result, vec3(1.0), greaterThanEqual(b, vec3(1.0)));
}

vec3 SoftLight(vec3 b, vec3 s)
{
  vec3 d = mix(sqrt(b), ((16.0 * b - 12.0) * b + 4.0) * b, lessThanEqual(b, vec3(0.25)));
  return mix(b + (2.0 * s - 1.0) * (d - b), b - (1.0 - 2.0 * s) * b * (1.0 - b), lessThanEqual(s, vec3(0.5)));
}

float Lum(vec3 c)
{
  return dot(c, vec3(0.3, 0.59, 0.11));
}

vec3 ClipColour(vec3 c)
{
  float l = Lum(c);
  float n = min(c.r, min(c.g, c.b));
  float x = max(c.r, max(c.g, c.b));
  if (n < 0.0)
  {
    c = l + (c - l) * l / (l - n);
  }
  if (x > 1.0)
  {
    c = l + (c - l) * (1.0 - l) / (x - l);
  }
  return c;
}

vec3 SetLum(vec3 c, float l)
{
  return ClipColour(c + (l - Lum(c)));
}

float Sat(vec3 c)
{
  return max(c.r, max(c.g, c.b)) - min(c.r, min(c.g, c.b));
}

// Scales c so its largest channel minus its smallest is s, its smallest channel at 0.
vec3 SetSat(vec3 c, float s)
{
  float n = min(c.r, min(c.g, c.b));
  float x = max(c.r, max(c.g, c.b));
  return x > n ? (c - n) * s / (x - n) : vec3(0.0);
}

vec3 Blend(uint mode, vec3 b, vec3 s)
{
  switch (mode)
  {
    case BlendMultiply: return b * s;
    case BlendScreen: return b + s - b * s;
    case BlendDarken: return min(b, s);
    case BlendLighten: return max(b, s);
    case BlendOverlay: return HardLight(s, b);
    case BlendColorDodge: return ColorDodge(b, s);
    case BlendColorBurn: return ColorBurn(b, s);
    case BlendHardLight: return HardLight(b, s);
    case BlendSoftLight: return SoftLight(b, s);
    case BlendDifference: return abs(b - s);
    case BlendExclusion: return b + s - 2.0 * b * s;
    case BlendHue: return SetLum(SetSat(s, Sat(b)), Lum(b));
    case BlendSaturation: return SetLum(SetSat(b, Sat(s)), Lum(b));
    case BlendColor: return SetLum(s, Lum(b));
    case BlendLuminosity: return SetLum(b, Lum(s));
    default: return s;
  }
}

void main()
{
  // Derivatives are taken here, outside the branch, where every fragment of the quad runs them.
  vec2 dx = dFdx(v_UV);
  vec2 dy = dFdy(v_UV);

  float coverage;
  if ((v_Flags & Shadow) != 0u)
  {
    // Blurring is linear, so a box with a hole blurs as the box's blur less the hole's.
    float sigma = v_UVRect.x;
    coverage = BlurredBox(v_Position, v_HalfSize, v_Radii, sigma);
    if ((v_Flags & Hole) != 0u)
    {
      coverage = max(coverage - BlurredBox(v_Position - v_Hole.xy, v_Hole.zw, v_HoleRadii, sigma), 0.0);
    }
  }
  else
  {
    coverage = Coverage(v_Position, v_HalfSize, v_Radii, v_InverseTranspose);
    if ((v_Flags & Hole) != 0u)
    {
      coverage *= 1.0 - Coverage(v_Position - v_Hole.xy, v_Hole.zw, v_HoleRadii, v_InverseTranspose);
    }
  }
  coverage *= ClipCoverage();

  vec4 colour = v_Colour;
  if ((v_Flags & Textured) != 0u)
  {
    vec4 texel = SampleTexture(dx, dy);
    if ((v_Flags & ColourMatrixFilter) != 0u)
    {
      vec4 straight = vec4(texel.a > 0.0 ? texel.rgb / texel.a : vec3(0.0), texel.a);
      vec4 filtered = clamp(u_ColourMatrix * straight + u_ColourMatrixOffset, 0.0, 1.0);
      texel = vec4(filtered.rgb * filtered.a, filtered.a);
    }
    colour *= texel;
  }
  if ((v_Flags & GradientPaint) != 0u)
  {
    // Dithered by under half a step either way, scaled by alpha so the colour stays premultiplied.
    vec4 gradient = GradientColour();
    gradient.rgb = clamp(gradient.rgb + (Bayer() - 0.5) * u_Dither * gradient.a, 0.0, gradient.a);
    colour *= gradient;
  }
  if ((v_Flags & PatternPaint) != 0u)
  {
    colour *= PatternColour();
  }
  vec4 source = colour * coverage;

  // Blend modes mix the source's colour with the backdrop's; normal blending then adds the backdrop
  // through what the source leaves, so where the draw covers nothing, nothing changes.
  uint blend = (v_Flags >> BlendModeShift) & 31u;
  if (blend >= BlendMultiply && blend <= BlendLuminosity && source.a > 0.0)
  {
    vec4 backdrop = texelFetch(u_Backdrop, ivec2(floor(v_Pixel - u_Origin)), 0);
    vec3 b = backdrop.a > 0.0 ? clamp(backdrop.rgb / backdrop.a, 0.0, 1.0) : vec3(0.0);
    vec3 s = clamp(colour.rgb / colour.a, 0.0, 1.0);
    vec3 mixed = mix(s, clamp(Blend(blend, b, s), 0.0, 1.0), backdrop.a);
    source.rgb = mixed * source.a;
  }
  o_Colour = source;
}
)";

    // Blurs a layer's texture along one axis: a Gaussian over the texels of the region, premultiplied. Texels
    // outside the region are transparent.
    constexpr const char8_t *BlurVertexSource = u8R"(#version 450
layout(binding = 0, std140) uniform Blur
{
  vec4 u_Region;      // the texels written and read: x, y, width, height
  vec2 u_TextureSize; // the destination's size
  vec2 u_Direction;   // 1, 0 across; 0, 1 down
  float u_Sigma;      // the standard deviation, in texels
  int u_Reach;        // how many texels either side are read
};

layout(location = 0) out vec2 v_Texel;

void main()
{
  vec2 corner = vec2(gl_VertexID & 1, gl_VertexID >> 1);
  vec2 texel = u_Region.xy + corner * u_Region.zw;
  vec2 ndc = texel / u_TextureSize * 2.0 - 1.0;
  gl_Position = vec4(ndc.x, -ndc.y, 0.0, 1.0);
  v_Texel = texel;
}
)";

    constexpr const char8_t *BlurFragmentSource = u8R"(#version 450
layout(binding = 0, std140) uniform Blur
{
  vec4 u_Region;
  vec2 u_TextureSize;
  vec2 u_Direction;
  float u_Sigma;
  int u_Reach;
};

layout(binding = 0) uniform sampler2D u_Source;

layout(location = 0) in vec2 v_Texel;
layout(location = 0) out vec4 o_Colour;

void main()
{
  ivec2 texel = ivec2(floor(v_Texel));
  ivec2 low = ivec2(u_Region.xy);
  ivec2 high = low + ivec2(u_Region.zw);
  ivec2 step = ivec2(u_Direction);
  float k = -0.5 / (u_Sigma * u_Sigma);
  vec4 sum = vec4(0.0);
  float total = 0.0;
  for (int i = -u_Reach; i <= u_Reach; ++i)
  {
    float weight = exp(float(i * i) * k);
    total += weight;
    ivec2 at = texel + step * i;
    if (all(greaterThanEqual(at, low)) && all(lessThan(at, high)))
    {
      sum += weight * texelFetch(u_Source, at, 0);
    }
  }
  o_Colour = sum / total;
}
)";

    /// @brief How far a blur of `sigma` reaches past what's drawn, in whole pixels: three standard
    /// deviations, where it fades out.
    KRYS_NODISCARD int32 BlurReach(float sigma) noexcept
    {
      // About three standard deviations of 85 pixels: the most texels either side of a pixel a blur reads.
      constexpr int32 maxBlurReach = 256;
      return sigma > 0.f ? std::min(static_cast<int32>(std::ceil(3.f * sigma)), maxBlurReach) : 0;
    }

    /// @brief The rect grown by `amount` pixels on every side.
    KRYS_NODISCARD PixelRect2D Inflate(const PixelRect2D &rect, int32 amount) noexcept
    {
      if (rect.IsEmpty())
      {
        return rect;
      }
      return {.X = rect.X - amount,
              .Y = rect.Y - amount,
              .Width = rect.Width + 2u * static_cast<uint32>(amount),
              .Height = rect.Height + 2u * static_cast<uint32>(amount)};
    }

    /// @brief Clamps negative radii to zero and scales radii that don't fit down together, as CSS does.
    KRYS_NODISCARD CornerRadii2D ConstrainRadii(const Rect2D &rect, CornerRadii2D radii) noexcept
    {
      radii.TopLeft = std::max(radii.TopLeft, 0.f);
      radii.TopRight = std::max(radii.TopRight, 0.f);
      radii.BottomRight = std::max(radii.BottomRight, 0.f);
      radii.BottomLeft = std::max(radii.BottomLeft, 0.f);

      float scale = 1.f;
      auto fit = [&](float length, float a, float b)
      {
        if (a + b > length)
        {
          scale = std::min(scale, length / (a + b));
        }
      };
      fit(rect.Width, radii.TopLeft, radii.TopRight);
      fit(rect.Width, radii.BottomLeft, radii.BottomRight);
      fit(rect.Height, radii.TopLeft, radii.BottomLeft);
      fit(rect.Height, radii.TopRight, radii.BottomRight);

      if (scale < 1.f)
      {
        radii.TopLeft *= scale;
        radii.TopRight *= scale;
        radii.BottomRight *= scale;
        radii.BottomLeft *= scale;
      }
      return radii;
    }
  }

  Expected<UniquePtr<Renderer2D>> Renderer2D::Create(Device &device, const Renderer2DDesc &desc) noexcept
  {
    auto renderer = CreateUnique<Renderer2D>(device, desc);
    if (auto result = renderer->Initialise(); !result)
    {
      return Unexpected<string>(result.error());
    }
    return renderer;
  }

  Renderer2D::Renderer2D(Device &device, const Renderer2DDesc &desc) noexcept
      : _device(&device), _masks(device, desc.Masks), _glyphs(device, desc.Glyphs)
  {
  }

  Renderer2D::~Renderer2D() noexcept
  {
    for (const auto &entry : _pipelines)
    {
      _device->Destroy(entry.Pipeline);
    }
    for (const auto &entry : _blurPipelines)
    {
      _device->Destroy(entry.Pipeline);
    }
    for (auto shader : {_blurFragmentShader, _blurVertexShader})
    {
      if (shader.IsValid())
      {
        _device->Destroy(shader);
      }
    }
    if (_fragmentShader.IsValid())
    {
      _device->Destroy(_fragmentShader);
    }
    if (_vertexShader.IsValid())
    {
      _device->Destroy(_vertexShader);
    }
    if (_nearestSampler.IsValid())
    {
      _device->Destroy(_nearestSampler);
    }
    if (_linearSampler.IsValid())
    {
      _device->Destroy(_linearSampler);
    }
    if (_whiteTexture.IsValid())
    {
      _device->Destroy(_whiteTexture);
    }
    for (const auto &texture : _layerTextures)
    {
      _device->Destroy(texture.Texture);
    }
  }

  Expected<void> Renderer2D::Initialise() noexcept
  {
    auto vertexShader = _device->CreateShader(
      {.Stage = ShaderStage::Vertex, .Source = RectVertexSource, .DebugName = u8"Render2D.Rect.vert"});
    if (!vertexShader)
    {
      return Unexpected<string>(vertexShader.error());
    }
    _vertexShader = *vertexShader;

    auto fragmentShader = _device->CreateShader(
      {.Stage = ShaderStage::Fragment, .Source = RectFragmentSource, .DebugName = u8"Render2D.Rect.frag"});
    if (!fragmentShader)
    {
      return Unexpected<string>(fragmentShader.error());
    }
    _fragmentShader = *fragmentShader;

    auto blurVertexShader = _device->CreateShader(
      {.Stage = ShaderStage::Vertex, .Source = BlurVertexSource, .DebugName = u8"Render2D.Blur.vert"});
    if (!blurVertexShader)
    {
      return Unexpected<string>(blurVertexShader.error());
    }
    _blurVertexShader = *blurVertexShader;

    auto blurFragmentShader = _device->CreateShader(
      {.Stage = ShaderStage::Fragment, .Source = BlurFragmentSource, .DebugName = u8"Render2D.Blur.frag"});
    if (!blurFragmentShader)
    {
      return Unexpected<string>(blurFragmentShader.error());
    }
    _blurFragmentShader = *blurFragmentShader;

    auto linearSampler = _device->CreateSampler({.MinFilter = SamplerFilter::Linear,
                                                 .MagFilter = SamplerFilter::Linear,
                                                 .MipmapMode = SamplerMipmapMode::Linear});
    if (!linearSampler)
    {
      return Unexpected<string>(linearSampler.error());
    }
    _linearSampler = *linearSampler;

    auto nearestSampler = _device->CreateSampler(SamplerDesc::NearestClamp());
    if (!nearestSampler)
    {
      return Unexpected<string>(nearestSampler.error());
    }
    _nearestSampler = *nearestSampler;

    auto repeatSampler = [&](SamplerFilter filter)
    {
      return _device->CreateSampler({.MinFilter = filter,
                                     .MagFilter = filter,
                                     .AddressU = SamplerAddressMode::Repeat,
                                     .AddressV = SamplerAddressMode::Repeat});
    };
    auto linearRepeatSampler = repeatSampler(SamplerFilter::Linear);
    if (!linearRepeatSampler)
    {
      return Unexpected<string>(linearRepeatSampler.error());
    }
    _linearRepeatSampler = *linearRepeatSampler;
    auto nearestRepeatSampler = repeatSampler(SamplerFilter::Nearest);
    if (!nearestRepeatSampler)
    {
      return Unexpected<string>(nearestRepeatSampler.error());
    }
    _nearestRepeatSampler = *nearestRepeatSampler;

    auto whiteTexture = _device->CreateTexture({.Width = 1u, .Height = 1u, .DebugName = u8"Render2D.White"});
    if (!whiteTexture)
    {
      return Unexpected<string>(whiteTexture.error());
    }
    _whiteTexture = *whiteTexture;
    constexpr Array<byte, 4> white {byte {255}, byte {255}, byte {255}, byte {255}};
    _device->WriteTexture(_whiteTexture, TextureRegion::Whole2D(1u, 1u), white);
    return {};
  }

  void Renderer2D::Begin(CommandBuffer &commands, const Render2DTarget &target) noexcept
  {
    krys_assert_msg(_commands == nullptr, "Begin called twice without End.");
    krys_assert_msg(target.Swapchain.IsValid() || target.Texture.IsValid(),
                    "The target needs a texture or swapchain.");
    krys_assert_msg(!IsSrgbFormat(target.Format), "Render2D blends encoded values, so targets are unorm.");
    _commands = &commands;
    _target = target;
    _state = {.Transform = {},
              .Clip = {.X = 0, .Y = 0, .Width = target.Width, .Height = target.Height},
              .Chain = NoChain,
              .Blend = BlendMode::Normal};
    _backdropCopies = 0u;
    _chains.clear();
    _savedStates.clear();
    _rects.clear();
    _batches.clear();
    _layers.clear();
    _layerOrder.clear();
    _currentLayer = NoLayer;
    _gradients.clear();
    _gradientStops.clear();
    _patterns.clear();
    _masks.ResetStats();
    _glyphs.ResetStats();

    // Layer textures unused for a while go.
    const uint64 frame = _device->FrameNumber();
    std::erase_if(_layerTextures,
                  [&](const LayerTexture &texture)
                  {
                    if (texture.LastUsedFrame + 120u < frame)
                    {
                      _device->Destroy(texture.Texture);
                      return true;
                    }
                    return false;
                  });
  }

  void Renderer2D::SetDropShadow(const DropShadow2D &shadow) noexcept
  {
    _state.DropShadow = shadow;
  }

  void Renderer2D::ClearDropShadow() noexcept
  {
    _state.DropShadow = {};
  }

  const Maybe<DropShadow2D> &Renderer2D::GetDropShadow() const noexcept
  {
    return _state.DropShadow;
  }

  void Renderer2D::SetShadowsIgnoreTransforms(bool ignore) noexcept
  {
    _state.ShadowsIgnoreTransforms = ignore;
  }

  bool Renderer2D::GetShadowsIgnoreTransforms() const noexcept
  {
    return _state.ShadowsIgnoreTransforms;
  }

  void Renderer2D::SetBlendMode(BlendMode mode) noexcept
  {
    krys_assert_msg(mode != BlendMode::PlusDarker, "BlendMode::PlusDarker isn't supported.");
    _state.Blend = mode == BlendMode::PlusDarker ? BlendMode::Normal : mode;
  }

  BlendMode Renderer2D::GetBlendMode() const noexcept
  {
    return _state.Blend;
  }

  void Renderer2D::BeginLayer(float opacity) noexcept
  {
    BeginLayer(opacity, _state.Blend);
  }

  void Renderer2D::BeginLayer(float opacity, BlendMode blend) noexcept
  {
    krys_debug_assert_msg(_commands != nullptr, "BeginLayer outside Begin and End.");
    krys_assert_msg(blend != BlendMode::PlusDarker, "BlendMode::PlusDarker isn't supported.");
    Save();
    _layers.push_back({.Parent = _currentLayer,
                       .Opacity = std::isfinite(opacity) ? std::clamp(opacity, 0.f, 1.f) : 0.f,
                       .Blend = blend == BlendMode::PlusDarker ? BlendMode::Normal : blend,
                       .Clip = _state.Clip,
                       .Bounds = {},
                       .SaveDepth = _savedStates.size()});
    _currentLayer = static_cast<uint32>(_layers.size() - 1u);
    _state.Blend = BlendMode::Normal;
  }

  void Renderer2D::BeginLayer(float opacity, BlendMode blend, const LayerFilter2D &filter) noexcept
  {
    BeginLayer(opacity, blend);
    _layers[_currentLayer].Filter = filter;
  }

  void Renderer2D::EndLayer() noexcept
  {
    krys_assert_msg(_currentLayer != NoLayer, "EndLayer without a matching BeginLayer.");
    if (_currentLayer == NoLayer)
    {
      return;
    }
    const uint32 index = _currentLayer;
    krys_debug_assert_msg(_savedStates.size() == _layers[index].SaveDepth,
                          "Save and Restore inside a layer don't match.");
    _currentLayer = _layers[index].Parent;
    _layerOrder.push_back(index);
    Restore();

    // The composite goes in the parent now, in painter's order; its texture is only known at End. The
    // layer's draws were already clipped, so the composite keeps only the scissor: clipping it again would
    // fade antialiased clip edges twice.
    if (_state.Clip.IsEmpty())
    {
      return;
    }
    auto &layer = _layers[index];
    if (layer.Filter && layer.Filter->Type == LayerFilter2D::Kind::DropShadow)
    {
      // The shadow is composited beneath the layer, from its own texture.
      layer.ShadowBatch = static_cast<uint32>(_batches.size());
      layer.ShadowInstance = static_cast<uint32>(_rects.size());
      _batches.push_back({.FirstInstance = layer.ShadowInstance,
                          .InstanceCount = 1u,
                          .Scissor = _state.Clip,
                          .Chain = NoChain,
                          .Texture = {},
                          .Sampler = {},
                          .Layer = _currentLayer,
                          .Composite = index,
                          .Blend = layer.Blend,
                          .Backdrop = {}});
      _rects.push_back({.Rect = {},
                        .Radii = {},
                        .Linear = {1.f, 0.f, 0.f, 1.f},
                        .Translation = {},
                        .UV = {},
                        .HoleRect = {},
                        .HoleRadii = {},
                        .Colour = {},
                        .Flags = static_cast<uint32>(layer.Blend) << BlendModeShift});
    }
    layer.CompositeBatch = static_cast<uint32>(_batches.size());
    layer.CompositeInstance = static_cast<uint32>(_rects.size());
    layer.HasComposite = true;
    _batches.push_back({.FirstInstance = layer.CompositeInstance,
                        .InstanceCount = 1u,
                        .Scissor = _state.Clip,
                        .Chain = NoChain,
                        .Texture = {},
                        .Sampler = {},
                        .Layer = _currentLayer,
                        .Composite = index,
                        .Blend = layer.Blend,
                        .Backdrop = {}});
    _rects.push_back({.Rect = {},
                      .Radii = {},
                      .Linear = {1.f, 0.f, 0.f, 1.f},
                      .Translation = {},
                      .UV = {},
                      .HoleRect = {},
                      .HoleRadii = {},
                      .Colour = {},
                      .Flags = static_cast<uint32>(layer.Blend) << BlendModeShift});
  }

  void Renderer2D::Save() noexcept
  {
    _savedStates.push_back(_state);
  }

  void Renderer2D::Restore() noexcept
  {
    krys_assert_msg(!_savedStates.empty(), "Restore without a matching Save.");
    if (!_savedStates.empty())
    {
      _state = _savedStates.back();
      _savedStates.pop_back();
    }
  }

  void Renderer2D::ConcatTransform(const Transform2D &transform) noexcept
  {
    _state.Transform = _state.Transform * transform;
  }

  void Renderer2D::Translate(float x, float y) noexcept
  {
    ConcatTransform(Transform2D::Translation(x, y));
  }

  void Renderer2D::Scale(float x, float y) noexcept
  {
    ConcatTransform(Transform2D::Scale(x, y));
  }

  void Renderer2D::Rotate(float radians) noexcept
  {
    ConcatTransform(Transform2D::Rotation(radians));
  }

  void Renderer2D::SetTransform(const Transform2D &transform) noexcept
  {
    _state.Transform = transform;
  }

  const Transform2D &Renderer2D::GetTransform() const noexcept
  {
    return _state.Transform;
  }

  void Renderer2D::ClipRect(const Rect2D &rect) noexcept
  {
    // On whole pixels and axis-aligned, the scissor alone is the clip.
    const auto &transform = _state.Transform;
    if (transform.PreservesAxisAlignment())
    {
      const auto a = transform.Apply({rect.X, rect.Y});
      const auto b = transform.Apply({rect.X + rect.Width, rect.Y + rect.Height});
      const float left = std::min(a.x, b.x);
      const float top = std::min(a.y, b.y);
      const float right = std::max(a.x, b.x);
      const float bottom = std::max(a.y, b.y);
      if (left == std::floor(left) && top == std::floor(top) && right == std::floor(right)
          && bottom == std::floor(bottom))
      {
        _state.Clip = Intersect(_state.Clip, ToPixelRect(left, top, right, bottom));
        return;
      }
    }
    AddClip({.Rect = rect, .Radii = {}}, false);
  }

  void Renderer2D::ClipRoundedRect(const RoundedRect2D &rect) noexcept
  {
    if (rect.Radii == CornerRadii2D {})
    {
      ClipRect(rect.Rect);
      return;
    }
    AddClip(rect, false);
  }

  void Renderer2D::ClipOutRect(const Rect2D &rect) noexcept
  {
    AddClip({.Rect = rect, .Radii = {}}, true);
  }

  void Renderer2D::ClipOutRoundedRect(const RoundedRect2D &rect) noexcept
  {
    AddClip(rect, true);
  }

  void Renderer2D::ClipPath(const Path2D &path, FillRule rule) noexcept
  {
    AddPathClip(path, rule, false);
  }

  void Renderer2D::ClipOutPath(const Path2D &path, FillRule rule) noexcept
  {
    AddPathClip(path, rule, true);
  }

  void Renderer2D::AddPathClip(const Path2D &path, FillRule rule, bool out) noexcept
  {
    // Clipping to nothing leaves nothing; clipping nothing out changes nothing.
    if (path.IsEmpty() || _state.Clip.IsEmpty() || !_state.Transform.Inverse())
    {
      if (!out)
      {
        _state.Clip = {};
      }
      return;
    }

    ClipChain chain = _state.Chain != NoChain ? _chains[_state.Chain] : ClipChain {};
    chain.Paths.push_back({.Path = path, .Transform = _state.Transform, .Rule = rule, .Out = out});

    if (chain.Paths.size() == 1u)
    {
      const auto mask = _masks.GetMask(path, _state.Transform, rule, _state.Clip);
      if (mask.Bounds.IsEmpty())
      {
        if (!out)
        {
          _state.Clip = {};
        }
        return;
      }
      if (!out)
      {
        _state.Clip = Intersect(_state.Clip, mask.Bounds);
      }
      chain.Mask = mask;
      chain.MaskOut = out;
    }
    else
    {
      // Several path clips multiply into one mask over the clip's bounds, for this frame.
      const auto region = _state.Clip;
      const size_t pixelCount = static_cast<size_t>(region.Width) * region.Height;
      _combinedClip.assign(pixelCount, 1.f);
      for (const auto &clip : chain.Paths)
      {
        const auto coverage = _rasteriser.Rasterise(clip.Path, clip.Transform, clip.Rule, region);
        for (uint32 y = 0u; y < region.Height; ++y)
        {
          for (uint32 x = 0u; x < region.Width; ++x)
          {
            const int32 pixelX = region.X + static_cast<int32>(x);
            const int32 pixelY = region.Y + static_cast<int32>(y);
            const auto &bounds = coverage.Bounds;
            const bool covered = pixelX >= bounds.X && pixelY >= bounds.Y
                                 && pixelX < bounds.X + static_cast<int32>(bounds.Width)
                                 && pixelY < bounds.Y + static_cast<int32>(bounds.Height);
            const float inside = covered ? static_cast<float>(coverage.At(pixelX, pixelY)) / 255.f : 0.f;
            _combinedClip[static_cast<size_t>(y) * region.Width + x] *= clip.Out ? 1.f - inside : inside;
          }
        }
      }

      CoverageMask combined {.Bounds = region, .Coverage = {}};
      combined.Coverage.resize(pixelCount);
      for (size_t i = 0u; i < pixelCount; ++i)
      {
        combined.Coverage[i] = static_cast<uint8>(std::clamp(_combinedClip[i], 0.f, 1.f) * 255.f + 0.5f);
      }
      chain.Mask = _masks.UploadTransient(combined);
      chain.MaskOut = false;
    }

    _chains.push_back(krys::move(chain));
    _state.Chain = static_cast<uint32>(_chains.size() - 1u);
  }

  void Renderer2D::AddClip(RoundedRect2D shape, bool out) noexcept
  {
    // Clipping to nothing leaves nothing; clipping nothing out changes nothing.
    const auto inverse = _state.Transform.Inverse();
    if (!inverse || shape.Rect.Width <= 0.f || shape.Rect.Height <= 0.f)
    {
      if (!out)
      {
        _state.Clip = {};
      }
      return;
    }
    shape.Radii = ConstrainRadii(shape.Rect, shape.Radii);

    if (!out)
    {
      // The scissor covers every pixel the shape's antialiased edge reaches.
      const auto &transform = _state.Transform;
      const auto &rect = shape.Rect;
      const Array<Vec2f, 4> corners {transform.Apply({rect.X, rect.Y}),
                                     transform.Apply({rect.X + rect.Width, rect.Y}),
                                     transform.Apply({rect.X, rect.Y + rect.Height}),
                                     transform.Apply({rect.X + rect.Width, rect.Y + rect.Height})};
      float left = corners[0].x;
      float top = corners[0].y;
      float right = corners[0].x;
      float bottom = corners[0].y;
      for (const auto &corner : corners)
      {
        left = std::min(left, corner.x);
        top = std::min(top, corner.y);
        right = std::max(right, corner.x);
        bottom = std::max(bottom, corner.y);
      }
      const auto x0 = static_cast<int32>(std::floor(left));
      const auto y0 = static_cast<int32>(std::floor(top));
      const auto x1 = static_cast<int32>(std::ceil(right));
      const auto y1 = static_cast<int32>(std::ceil(bottom));
      _state.Clip = Intersect(_state.Clip, {.X = x0,
                                            .Y = y0,
                                            .Width = static_cast<uint32>(std::max(x1 - x0, 0)),
                                            .Height = static_cast<uint32>(std::max(y1 - y0, 0))});
    }

    ClipChain chain = _state.Chain != NoChain ? _chains[_state.Chain] : ClipChain {};
    krys_assert_msg(chain.Shapes.size() < MaxClips, "Too many shape clips at once.");
    if (chain.Shapes.size() >= MaxClips)
    {
      return;
    }
    chain.Shapes.push_back({.Shape = shape, .Inverse = *inverse, .Out = out});
    _chains.push_back(krys::move(chain));
    _state.Chain = static_cast<uint32>(_chains.size() - 1u);
  }

  void Renderer2D::FillRect(const Rect2D &rect, const Paint2D &paint) noexcept
  {
    FillRoundedRect({.Rect = rect, .Radii = {}}, paint);
  }

  void Renderer2D::FillRoundedRect(const RoundedRect2D &roundedRect, const Paint2D &paint) noexcept
  {
    krys_debug_assert_msg(_commands != nullptr, "FillRoundedRect outside Begin and End.");
    const auto &rect = roundedRect.Rect;
    if (rect.Width <= 0.f || rect.Height <= 0.f || _state.Clip.IsEmpty())
    {
      return;
    }

    if (const auto resolved = ResolvePaint(paint))
    {
      AddRoundedRect(roundedRect, *resolved, _state.Transform);
    }
  }

  void Renderer2D::AddRoundedRect(const RoundedRect2D &roundedRect, const ResolvedPaint &paint,
                                  const Transform2D &transform) noexcept
  {
    const auto &rect = roundedRect.Rect;
    if (rect.Width <= 0.f || rect.Height <= 0.f)
    {
      return;
    }
    const auto radii = ConstrainRadii(rect, roundedRect.Radii);
    RectInstance instance {.Rect = {rect.X, rect.Y, rect.Width, rect.Height},
                           .Radii = {radii.TopLeft, radii.TopRight, radii.BottomRight, radii.BottomLeft},
                           .Linear = {},
                           .Translation = {},
                           .UV = {},
                           .HoleRect = {},
                           .HoleRadii = {},
                           .Colour = paint.Colour,
                           .Flags = 0u};
    if (const auto shadow = PlaceShadow(transform, paint))
    {
      auto cast = instance;
      cast.UV = {shadow->Sigma, 0.f, 0.f, 0.f};
      cast.Colour = shadow->Colour;
      cast.Flags = shadow->Sigma > 0.f ? Shadow : 0u;
      AddInstance(cast, shadow->Transform, {}, {});
    }
    AddInstance(instance, transform, {}, {}, paint.Gradient, paint.Pattern);
  }

  void Renderer2D::StrokePath(const Path2D &path, const StrokeStyle &style, const Paint2D &paint) noexcept
  {
    krys_debug_assert_msg(_commands != nullptr, "StrokePath outside Begin and End.");
    if (IsInvisible(paint) || path.IsEmpty() || _state.Clip.IsEmpty())
    {
      return;
    }
    const auto outline = _stroker.Stroke(path, style, _state.Transform);
    if (!outline.IsEmpty())
    {
      FillPath(outline, FillRule::NonZero, paint);
    }
  }

  void Renderer2D::StrokeLine(Vec2f from, Vec2f to, const StrokeStyle &style, const Paint2D &paint) noexcept
  {
    krys_debug_assert_msg(_commands != nullptr, "StrokeLine outside Begin and End.");
    if (IsInvisible(paint) || _state.Clip.IsEmpty())
    {
      return;
    }

    // A hairline is one target pixel wide, whatever the transform's scale.
    const float halfWidth =
      style.Width > 0.f ? style.Width * 0.5f : 0.5f / std::sqrt(std::abs(_state.Transform.Determinant()));
    if (!(halfWidth > 0.f) || !std::isfinite(halfWidth))
    {
      return;
    }

    // The line's own coordinates: x along it from `from`, y across it. A zero-length line lies along x.
    const float dx = to.x - from.x;
    const float dy = to.y - from.y;
    const float length = std::sqrt(dx * dx + dy * dy);
    const Vec2f along = length > 0.f ? Vec2f {dx / length, dy / length} : Vec2f {1.f, 0.f};
    const auto transform =
      _state.Transform
      * Transform2D {.A = along.x, .B = along.y, .C = -along.y, .D = along.x, .E = from.x, .F = from.y};

    // Square and round caps reach half the width past each end; round ones are the rect's rounded ends.
    const float extension = style.Cap == LineCap::Butt ? 0.f : halfWidth;
    const float radius = style.Cap == LineCap::Round ? halfWidth : 0.f;
    const auto resolved = ResolvePaint(paint);
    if (!resolved)
    {
      return;
    }
    auto addDash = [&](float start, float end)
    {
      AddRoundedRect({.Rect = {start - extension, -halfWidth, end - start + 2.f * extension, 2.f * halfWidth},
                      .Radii = CornerRadii2D::Uniform(radius)},
                     *resolved, transform);
    };

    detail::PrepareDashes(style, _dashes);
    if (_dashes.empty() || length / detail::PatternLength(_dashes) > static_cast<float>(detail::MaxDashes))
    {
      addDash(0.f, length);
      return;
    }

    // As the stroker does: a dash ending exactly at the end is followed there by any dash starting there.
    auto position = detail::StartDashes(_dashes, style.DashOffset);
    float start = 0.f;
    for (;;)
    {
      const float end = start + position.Remaining;
      if (position.On)
      {
        addDash(start, std::min(end, length));
      }
      if (end > length)
      {
        break;
      }
      start = end;
      position.Next(_dashes);
    }
  }

  void Renderer2D::FillRectWithRoundedHole(const Rect2D &rect, const RoundedRect2D &hole,
                                           const Paint2D &paint) noexcept
  {
    krys_debug_assert_msg(_commands != nullptr, "FillRectWithRoundedHole outside Begin and End.");
    const auto &holeRect = hole.Rect;
    if (holeRect.Width <= 0.f || holeRect.Height <= 0.f)
    {
      FillRect(rect, paint);
      return;
    }
    if (rect.Width <= 0.f || rect.Height <= 0.f || _state.Clip.IsEmpty())
    {
      return;
    }

    const auto resolved = ResolvePaint(paint);
    if (!resolved)
    {
      return;
    }

    const auto radii = ConstrainRadii(holeRect, hole.Radii);
    RectInstance instance {.Rect = {rect.X, rect.Y, rect.Width, rect.Height},
                           .Radii = {},
                           .Linear = {},
                           .Translation = {},
                           .UV = {},
                           .HoleRect = {holeRect.X, holeRect.Y, holeRect.Width, holeRect.Height},
                           .HoleRadii = {radii.TopLeft, radii.TopRight, radii.BottomRight, radii.BottomLeft},
                           .Colour = resolved->Colour,
                           .Flags = Hole};
    if (const auto shadow = PlaceShadow(_state.Transform, *resolved))
    {
      auto cast = instance;
      cast.UV = {shadow->Sigma, 0.f, 0.f, 0.f};
      cast.Colour = shadow->Colour;
      cast.Flags = shadow->Sigma > 0.f ? Hole | Shadow : Hole;
      AddInstance(cast, shadow->Transform, {}, {});
    }
    AddInstance(instance, _state.Transform, {}, {}, resolved->Gradient, resolved->Pattern);
  }

  void Renderer2D::DrawImage(TextureHandle texture, const Rect2D &source, const Rect2D &destination,
                             const ImageOptions &options) noexcept
  {
    krys_debug_assert_msg(_commands != nullptr, "DrawImage outside Begin and End.");
    const auto desc = _device->GetTextureDesc(texture);
    krys_debug_assert_msg(desc.has_value(), "Drawing a stale or invalid texture.");
    if (!desc || source.Width <= 0.f || source.Height <= 0.f || destination.Width <= 0.f
        || destination.Height <= 0.f)
    {
      return;
    }

    const auto alpha = static_cast<uint8>(std::clamp(options.Opacity, 0.f, 1.f) * 255.f + 0.5f);
    if (alpha == 0u)
    {
      return;
    }

    const float width = static_cast<float>(desc->Width);
    const float height = static_cast<float>(desc->Height);
    uint32 flags = Textured;
    if (options.Alpha == AlphaPremultiplication::Unpremultiplied)
    {
      flags |= StraightAlpha;
    }
    if (IsSrgbFormat(desc->Format))
    {
      flags |= LinearTexels;
    }

    AddInstance({.Rect = {destination.X, destination.Y, destination.Width, destination.Height},
                 .Radii = {},
                 .Linear = {},
                 .Translation = {},
                 .UV = {source.X / width, source.Y / height, (source.X + source.Width) / width,
                        (source.Y + source.Height) / height},
                 .HoleRect = {},
                 .HoleRadii = {},
                 .Colour = {255u, 255u, 255u, alpha},
                 .Flags = flags},
                _state.Transform, texture,
                options.Filter == SamplerFilter::Nearest ? _nearestSampler : _linearSampler);
  }

  void Renderer2D::FillPath(const Path2D &path, FillRule rule, const Paint2D &paint) noexcept
  {
    krys_debug_assert_msg(_commands != nullptr, "FillPath outside Begin and End.");
    if (IsInvisible(paint) || path.IsEmpty() || _state.Clip.IsEmpty())
    {
      return;
    }

    const auto resolved = ResolvePaint(paint);
    if (!resolved)
    {
      return;
    }

    FillPathMask(path, _state.Transform, rule, *resolved);
  }

  void Renderer2D::FillPathMask(const Path2D &path, const Transform2D &geometry, FillRule rule,
                                const ResolvedPaint &paint) noexcept
  {
    // The shadow's mask is the path's, moved and blurred in target pixels. It can show where the path
    // doesn't, so it comes first.
    if (const auto shadow = PlaceShadow(geometry, paint))
    {
      AddMask(_masks.GetMask(path, shadow->Transform, rule, _state.Clip, shadow->PixelSigma), shadow->Colour,
              NoGradient);
    }
    AddMask(_masks.GetMask(path, geometry, rule, _state.Clip), paint.Colour, paint.Gradient, paint.Pattern);
  }

  void Renderer2D::AddMask(const MaskLocation &mask, Array<uint8, 4> colour, uint32 gradient,
                           uint32 pattern) noexcept
  {
    if (mask.Bounds.IsEmpty() || Intersect(mask.Bounds, _state.Clip).IsEmpty())
    {
      return;
    }
    const auto desc = _device->GetTextureDesc(mask.Texture);
    if (!desc)
    {
      return;
    }

    // The mask is already in target pixels, so it's drawn untransformed, on whole pixels.
    const float width = static_cast<float>(desc->Width);
    const float height = static_cast<float>(desc->Height);
    const auto &bounds = mask.Bounds;
    const auto &texels = mask.Texels;
    AddInstance({.Rect = {static_cast<float>(bounds.X), static_cast<float>(bounds.Y),
                          static_cast<float>(bounds.Width), static_cast<float>(bounds.Height)},
                 .Radii = {},
                 .Linear = {},
                 .Translation = {},
                 .UV = {static_cast<float>(texels.X) / width, static_cast<float>(texels.Y) / height,
                        static_cast<float>(texels.X + static_cast<int32>(texels.Width)) / width,
                        static_cast<float>(texels.Y + static_cast<int32>(texels.Height)) / height},
                 .HoleRect = {},
                 .HoleRadii = {},
                 .Colour = colour,
                 .Flags = Textured | Mask},
                Transform2D::Identity(), mask.Texture, _nearestSampler, gradient, pattern);
  }

  void Renderer2D::DrawGlyphs(const FontFace2D &face, float size, Span<const uint32> glyphs,
                              Span<const Vec2f> positions, Vec2f origin, const Paint2D &paint) noexcept
  {
    krys_debug_assert_msg(_commands != nullptr, "DrawGlyphs outside Begin and End.");
    krys_assert_msg(glyphs.size() == positions.size(), "Every glyph needs a position.");
    const size_t count = std::min(glyphs.size(), positions.size());
    if (count == 0u || IsInvisible(paint) || _state.Clip.IsEmpty() || !(size > 0.f) || !std::isfinite(size))
    {
      return;
    }
    const auto resolved = ResolvePaint(paint);
    const auto &transform = _state.Transform;
    const float scale = std::sqrt(std::abs(transform.Determinant()));
    if (!resolved || !(scale > 0.f) || !std::isfinite(scale))
    {
      return;
    }
    auto pen = [&](size_t i)
    {
      return Vec2f {origin.x + positions[i].x, origin.y + positions[i].y};
    };

    // A glyph's outline, at the origin, placed at its pen by the transform, so moving it by whole pixels
    // reuses its mask.
    auto fillOutline = [&](size_t i)
    {
      const auto path = face.GlyphPath(glyphs[i], size);
      if (!path.IsEmpty())
      {
        const auto at = pen(i);
        FillPathMask(path, transform * Transform2D::Translation(at.x, at.y), FillRule::NonZero, *resolved);
      }
    };

    // Large text is filled as outlines, shadows and all; colour glyphs only come as bitmaps.
    if (size * scale > LargeGlyphSize && !face.HasColour())
    {
      for (size_t i = 0u; i < count; ++i)
      {
        fillOutline(i);
      }
      return;
    }

    // Under a uniform scale, glyphs are rasterised at the scaled size, so hinting still applies; under any
    // other transform, they're rasterised under it.
    const bool uniform =
      transform.B == 0.f && transform.C == 0.f && transform.A == transform.D && transform.A > 0.f;
    const float rasterSize = uniform ? size * transform.A : size;
    const Transform2D linear =
      uniform ? Transform2D {}
              : Transform2D {.A = transform.A, .B = transform.B, .C = transform.C, .D = transform.D};

    // A glyph from the atlas, its pen in target pixels snapped to a quarter across and a whole pixel down;
    // false when the atlas has no room for it.
    auto drawGlyph = [&](uint32 glyph, Vec2f at, const ResolvedPaint &glyphPaint, float blur, bool shadow)
    {
      const auto [x, quarter] = GlyphAtlas::SnapPen(at.x);
      const auto y = static_cast<int32>(std::lround(at.y));
      if (const auto location = _glyphs.GetGlyph(face, glyph, rasterSize, quarter, linear, blur))
      {
        if (location->Texture.IsValid())
        {
          AddGlyph(*location, x, y, glyphPaint, shadow);
        }
        return true;
      }
      return false;
    };

    // Shadows go beneath the whole run: each glyph's coverage (a colour glyph's alpha), blurred in target
    // pixels, at its pen moved by the offset, in the shadow's colour.
    if (const auto shadow = PlaceShadow(transform, *resolved))
    {
      const Vec2f offset {shadow->Transform.E - transform.E, shadow->Transform.F - transform.F};
      const ResolvedPaint shadowPaint {.Colour = shadow->Colour, .Gradient = NoGradient};
      for (size_t i = 0u; i < count; ++i)
      {
        const auto at = transform.Apply(pen(i));
        if (!drawGlyph(glyphs[i], {at.x + offset.x, at.y + offset.y}, shadowPaint, shadow->PixelSigma, true))
        {
          // No room: the outline's shadow alone.
          const auto path = face.GlyphPath(glyphs[i], size);
          const auto from = pen(i);
          if (!path.IsEmpty())
          {
            AddMask(_masks.GetMask(path, shadow->Transform * Transform2D::Translation(from.x, from.y),
                                   FillRule::NonZero, _state.Clip, shadow->PixelSigma),
                    shadow->Colour, NoGradient);
          }
        }
      }
    }

    for (size_t i = 0u; i < count; ++i)
    {
      if (!drawGlyph(glyphs[i], transform.Apply(pen(i)), *resolved, 0.f, false))
      {
        // No room in the atlas (or too large for a page): the glyph's outline instead.
        const auto path = face.GlyphPath(glyphs[i], size);
        const auto at = pen(i);
        if (!path.IsEmpty())
        {
          AddMask(_masks.GetMask(path, transform * Transform2D::Translation(at.x, at.y), FillRule::NonZero,
                                 _state.Clip),
                  resolved->Colour, resolved->Gradient, resolved->Pattern);
        }
      }
    }
  }

  void Renderer2D::AddGlyph(const GlyphLocation &glyph, int32 x, int32 y, const ResolvedPaint &paint,
                            bool shadow) noexcept
  {
    const auto desc = _device->GetTextureDesc(glyph.Texture);
    if (!desc)
    {
      return;
    }
    const float width = static_cast<float>(desc->Width);
    const float height = static_cast<float>(desc->Height);
    const auto &texels = glyph.Texels;
    const auto &bounds = glyph.Bounds;

    // Coverage multiplies the paint, as a mask does; colour glyphs are premultiplied images, faded by the
    // paint's alpha. A shadow is coverage in its colour: a colour glyph's alpha.
    const bool image = glyph.Colour && !shadow;
    const uint32 mask = glyph.Colour ? AlphaMask : Mask;
    const uint8 alpha = paint.Gradient != NoGradient ? 255u : paint.Colour[3];
    AddInstance({.Rect = {static_cast<float>(x + bounds.X), static_cast<float>(y + bounds.Y),
                          static_cast<float>(bounds.Width), static_cast<float>(bounds.Height)},
                 .Radii = {},
                 .Linear = {},
                 .Translation = {},
                 .UV = {static_cast<float>(texels.X) / width, static_cast<float>(texels.Y) / height,
                        static_cast<float>(texels.X + static_cast<int32>(texels.Width)) / width,
                        static_cast<float>(texels.Y + static_cast<int32>(texels.Height)) / height},
                 .HoleRect = {},
                 .HoleRadii = {},
                 .Colour = image ? Array<uint8, 4> {255u, 255u, 255u, alpha} : paint.Colour,
                 .Flags = image ? Textured : Textured | mask},
                Transform2D::Identity(), glyph.Texture, _nearestSampler, image ? NoGradient : paint.Gradient,
                image ? NoPattern : paint.Pattern);
  }

  void Renderer2D::DrawImage(TextureHandle texture, const Rect2D &destination,
                             const ImageOptions &options) noexcept
  {
    const auto desc = _device->GetTextureDesc(texture);
    krys_debug_assert_msg(desc.has_value(), "Drawing a stale or invalid texture.");
    if (desc)
    {
      DrawImage(texture, {0.f, 0.f, static_cast<float>(desc->Width), static_cast<float>(desc->Height)},
                destination, options);
    }
  }

  void Renderer2D::AddInstance(RectInstance instance, const Transform2D &transform, TextureHandle texture,
                               SamplerHandle sampler, uint32 gradient, uint32 pattern) noexcept
  {
    // Nothing shows through an empty clip or a transform that flattens everything.
    if (_state.Clip.IsEmpty() || transform.Determinant() == 0.f)
    {
      return;
    }
    instance.Linear = {transform.A, transform.B, transform.C, transform.D};
    instance.Translation = {transform.E, transform.F};
    instance.Flags |= static_cast<uint32>(_state.Blend) << BlendModeShift;
    if (gradient != NoGradient)
    {
      instance.Flags |= GradientPaint;
    }
    if (pattern != NoPattern)
    {
      instance.Flags |= PatternPaint;
    }

    // A layer is as large as what's drawn in it.
    const auto reach = Reach(instance, transform);
    if (_currentLayer != NoLayer)
    {
      auto &layer = _layers[_currentLayer];
      layer.Bounds = Union(layer.Bounds, reach);
    }

    // Untextured instances join any batch of their layer with the same scissor, chain and blend mode; a
    // textured one needs the batch's texture, or a batch that has none yet. Composites only take untextured
    // instances. Likewise, a gradient instance needs the batch's gradient, or a batch that has none yet. A
    // blend mode that reads the backdrop only joins a batch whose pixels it doesn't overlap, since the
    // batch's backdrop is copied before any of it is drawn.
    const bool textured = texture.IsValid();
    const bool copies = CopiesBackdrop(_state.Blend);
    auto joins = [&](const Batch &batch)
    {
      const bool batchTextured = batch.Texture.IsValid() || batch.Composite != NoLayer;
      return batch.Layer == _currentLayer && batch.Scissor == _state.Clip && batch.Chain == _state.Chain
             && batch.Blend == _state.Blend
             && (!textured || !batchTextured || (batch.Texture == texture && batch.Sampler == sampler))
             && (gradient == NoGradient || batch.Gradient == NoGradient || batch.Gradient == gradient)
             && (pattern == NoPattern || batch.Pattern == NoPattern || batch.Pattern == pattern)
             && (!copies || (batch.Composite == NoLayer && Intersect(batch.Backdrop, reach).IsEmpty()));
    };
    if (_batches.empty() || !joins(_batches.back()))
    {
      _batches.push_back({.FirstInstance = static_cast<uint32>(_rects.size()),
                          .InstanceCount = 0u,
                          .Scissor = _state.Clip,
                          .Chain = _state.Chain,
                          .Texture = {},
                          .Sampler = {},
                          .Layer = _currentLayer,
                          .Composite = NoLayer,
                          .Blend = _state.Blend,
                          .Backdrop = {}});
    }

    auto &batch = _batches.back();
    if (textured && !batch.Texture.IsValid() && batch.Composite == NoLayer)
    {
      batch.Texture = texture;
      batch.Sampler = sampler;
    }
    if (gradient != NoGradient)
    {
      batch.Gradient = gradient;
    }
    if (pattern != NoPattern)
    {
      batch.Pattern = pattern;
    }
    if (copies)
    {
      batch.Backdrop = Union(batch.Backdrop, reach);
    }
    ++batch.InstanceCount;
    _rects.push_back(instance);
  }

  Maybe<Renderer2D::ShadowPlacement> Renderer2D::PlaceShadow(const Transform2D &transform,
                                                             const ResolvedPaint &paint) const noexcept
  {
    if (!_state.DropShadow)
    {
      return {};
    }
    const auto &shadow = *_state.DropShadow;
    const auto colour = shadow.Colour.Resolved();
    const uint32 paintAlpha = paint.Gradient != NoGradient ? 255u : paint.Colour[3];
    const auto alpha = static_cast<uint8>((colour.Alpha * paintAlpha + 127u) / 255u);
    const float radius = std::isfinite(shadow.BlurRadius) ? std::max(shadow.BlurRadius, 0.f) : 0.f;
    Vec2f offset = shadow.Offset;
    if (alpha == 0u || !std::isfinite(offset.x) || !std::isfinite(offset.y)
        || (offset.x == 0.f && offset.y == 0.f && radius == 0.f))
    {
      return {};
    }

    // The offset and blur are in the current coordinates unless shadows ignore transforms; the blur is
    // measured in the shape's own units for the shader, and in target pixels for masks (where a transform
    // that stretches unevenly is taken by its average scale).
    const auto &current = _state.Transform;
    const float currentScale = std::sqrt(std::abs(current.Determinant()));
    const float localScale = std::sqrt(std::abs(transform.Determinant()));
    if (!(currentScale > 0.f) || !(localScale > 0.f))
    {
      return {};
    }
    float pixelSigma = radius * 0.5f;
    if (!_state.ShadowsIgnoreTransforms)
    {
      offset = {current.A * offset.x + current.C * offset.y, current.B * offset.x + current.D * offset.y};
      pixelSigma *= currentScale;
    }
    return ShadowPlacement {.Transform = Transform2D::Translation(offset.x, offset.y) * transform,
                            .Sigma = pixelSigma / localScale,
                            .PixelSigma = pixelSigma,
                            .Colour = {colour.Red, colour.Green, colour.Blue, alpha}};
  }

  bool Renderer2D::IsInvisible(const Paint2D &paint) noexcept
  {
    if (const auto *pattern = paint.GetPattern())
    {
      return !pattern->Texture.IsValid() || !(pattern->Opacity > 0.f);
    }
    const auto *gradient = paint.GetGradient();
    return gradient != nullptr ? gradient->Stops.empty() : paint.GetColour().Resolved().Alpha == 0u;
  }

  Maybe<Renderer2D::ResolvedPaint> Renderer2D::ResolvePattern(const ImagePattern2D &pattern) noexcept
  {
    const auto alpha = static_cast<uint8>(std::clamp(pattern.Opacity, 0.f, 1.f) * 255.f + 0.5f);
    const auto desc = _device->GetTextureDesc(pattern.Texture);
    krys_debug_assert_msg(desc.has_value(), "Painting with a stale or invalid texture.");
    if (alpha == 0u || !desc || !(pattern.TileSize.x > 0.f) || !(pattern.TileSize.y > 0.f)
        || pattern.Spacing.x < 0.f || pattern.Spacing.y < 0.f)
    {
      return {};
    }
    const auto inverse = (_state.Transform * pattern.PatternSpace).Inverse();
    if (!inverse)
    {
      return {};
    }

    PatternData data {
      .Texture = pattern.Texture,
      .Sampler = pattern.Filter == SamplerFilter::Nearest ? _nearestRepeatSampler : _linearRepeatSampler,
      .Inverse = *inverse,
      .TileAndSpacing = {pattern.TileSize.x, pattern.TileSize.y, pattern.Spacing.x, pattern.Spacing.y},
      .Flags = (pattern.Alpha == AlphaPremultiplication::Unpremultiplied ? PatternStraightAlpha : 0u)
               | (IsSrgbFormat(desc->Format) ? PatternLinearTexels : 0u)
               | (pattern.RepeatX ? 0u : PatternClampX) | (pattern.RepeatY ? 0u : PatternClampY)};
    const ResolvedPaint resolved {.Colour = {255u, 255u, 255u, alpha}, .Gradient = NoGradient, .Pattern = 0u};

    // Draws in a row with the same pattern share it, and so a batch.
    if (!_patterns.empty())
    {
      const auto &last = _patterns.back();
      if (last.Texture == data.Texture && last.Sampler == data.Sampler && last.Inverse == data.Inverse
          && last.TileAndSpacing == data.TileAndSpacing && last.Flags == data.Flags)
      {
        return ResolvedPaint {.Colour = resolved.Colour,
                              .Gradient = NoGradient,
                              .Pattern = static_cast<uint32>(_patterns.size() - 1u)};
      }
    }
    _patterns.push_back(data);
    return ResolvedPaint {.Colour = resolved.Colour,
                          .Gradient = NoGradient,
                          .Pattern = static_cast<uint32>(_patterns.size() - 1u)};
  }

  Maybe<Renderer2D::ResolvedPaint> Renderer2D::ResolvePaint(const Paint2D &paint) noexcept
  {
    if (const auto *pattern = paint.GetPattern())
    {
      return ResolvePattern(*pattern);
    }
    const auto *gradient = paint.GetGradient();
    if (gradient == nullptr)
    {
      const auto colour = paint.GetColour().Resolved();
      if (colour.Alpha == 0u)
      {
        return {};
      }
      return ResolvedPaint {.Colour = {colour.Red, colour.Green, colour.Blue, colour.Alpha},
                            .Gradient = NoGradient};
    }

    // A premultiplied colour, as an instance's straight colour.
    auto solid = [](const Array<float, 4> &colour) -> Maybe<ResolvedPaint>
    {
      const auto alpha = static_cast<uint8>(std::clamp(colour[3], 0.f, 1.f) * 255.f + 0.5f);
      if (alpha == 0u)
      {
        return {};
      }
      auto channel = [&](float value)
      {
        return static_cast<uint8>(std::clamp(value / colour[3], 0.f, 1.f) * 255.f + 0.5f);
      };
      return ResolvedPaint {.Colour = {channel(colour[0]), channel(colour[1]), channel(colour[2]), alpha},
                            .Gradient = NoGradient};
    };

    detail::FlattenGradientStops(*gradient, _flattened);
    if (_flattened.empty())
    {
      return {};
    }
    if (_flattened.size() == 1u)
    {
      return solid(_flattened.front().Colour);
    }

    // Each shape's own space, mapped into the gradient's: there, the shader's sums are short.
    constexpr float degenerateThreshold = 1.f / 32768.f;
    GradientData data {.Shape = LinearShape, .Spread = gradient->Spread};
    Transform2D shape {};
    bool degenerate = false;
    SwitchOn(
      gradient->Shape,
      [&](const LinearGradient2D &linear)
      {
        // The line runs along x from 0 to 1, and y across it.
        const float dx = linear.Point1.x - linear.Point0.x;
        const float dy = linear.Point1.y - linear.Point0.y;
        degenerate = !(std::sqrt(dx * dx + dy * dy) > degenerateThreshold);
        shape = {.A = dx, .B = dy, .C = -dy, .D = dx, .E = linear.Point0.x, .F = linear.Point0.y};
      },
      [&](const RadialGradient2D &radial)
      {
        // Elliptical gradients squash circles vertically about the first centre, which is the origin.
        const float aspectRatio = radial.AspectRatio;
        shape = Transform2D::Translation(radial.Point0.x, radial.Point0.y)
                * Transform2D::Scale(1.f, aspectRatio != 1.f ? 1.f / aspectRatio : 1.f);
        auto radius = [](float value)
        {
          return std::isfinite(value) ? std::max(value, 0.f) : 0.f;
        };
        const float startRadius = radius(radial.StartRadius);
        const float endRadius = radius(radial.EndRadius);
        const float dx = radial.Point1.x - radial.Point0.x;
        const float dy = radial.Point1.y - radial.Point0.y;
        data.Shape = RadialShape;
        data.Radial = {dx, dy, startRadius, endRadius};

        // Circles that coincide: a ring at the radius, so padded, the first colour inside it and the last
        // outside; otherwise nothing between the ends.
        if (std::sqrt(dx * dx + dy * dy) <= degenerateThreshold
            && std::abs(startRadius - endRadius) <= degenerateThreshold)
        {
          if (gradient->Spread == GradientSpreadMethod::Pad && endRadius > degenerateThreshold)
          {
            const auto first = _flattened.front().Colour;
            const auto last = _flattened.back().Colour;
            _flattened = {{.Offset = 0.f, .Colour = first},
                          {.Offset = 1.f, .Colour = first},
                          {.Offset = 1.f, .Colour = last}};
            data.Radial = {0.f, 0.f, 0.f, endRadius};
          }
          else
          {
            degenerate = true;
          }
        }
      },
      [&](const ConicGradient2D &conic)
      {
        // Upright at the start angle, then clockwise: from x in the shape's space.
        data.Shape = ConicShape;
        shape = Transform2D::Translation(conic.Point0.x, conic.Point0.y)
                * Transform2D::Rotation(conic.AngleRadians - 1.57079632679f);
      });

    if (degenerate)
    {
      return solid(gradient->Spread == GradientSpreadMethod::Pad ? _flattened.back().Colour
                                                                 : detail::AverageFlatStops(_flattened));
    }

    const auto inverse = (_state.Transform * paint.GetGradientSpace() * shape).Inverse();
    if (!inverse)
    {
      return {};
    }
    data.Inverse = *inverse;
    data.FirstStop = static_cast<uint32>(_gradientStops.size());
    data.StopCount = static_cast<uint32>(_flattened.size());

    // Draws in a row with the same gradient share it, and so a batch.
    if (!_gradients.empty())
    {
      const auto &last = _gradients.back();
      if (last.Shape == data.Shape && last.Spread == data.Spread && last.Inverse == data.Inverse
          && last.Radial == data.Radial && last.StopCount == data.StopCount
          && std::equal(_flattened.begin(), _flattened.end(), _gradientStops.begin() + last.FirstStop))
      {
        return ResolvedPaint {.Colour = {255u, 255u, 255u, 255u},
                              .Gradient = static_cast<uint32>(_gradients.size() - 1u)};
      }
    }
    _gradientStops.insert(_gradientStops.end(), _flattened.begin(), _flattened.end());
    _gradients.push_back(data);
    return ResolvedPaint {.Colour = {255u, 255u, 255u, 255u},
                          .Gradient = static_cast<uint32>(_gradients.size() - 1u)};
  }

  PixelRect2D Renderer2D::Reach(const RectInstance &instance, const Transform2D &transform) const noexcept
  {
    // A shadow reaches as far as its quad: three standard deviations past its shape.
    auto rect = instance.Rect;
    if ((instance.Flags & Shadow) != 0u)
    {
      const float reach = 3.f * instance.UV[0];
      rect = {rect[0] - reach, rect[1] - reach, rect[2] + 2.f * reach, rect[3] + 2.f * reach};
    }
    const Array<Vec2f, 4> corners {
      transform.Apply({rect[0], rect[1]}), transform.Apply({rect[0] + rect[2], rect[1]}),
      transform.Apply({rect[0], rect[1] + rect[3]}), transform.Apply({rect[0] + rect[2], rect[1] + rect[3]})};
    float left = corners[0].x;
    float top = corners[0].y;
    float right = corners[0].x;
    float bottom = corners[0].y;
    for (const auto &corner : corners)
    {
      left = std::min(left, corner.x);
      top = std::min(top, corner.y);
      right = std::max(right, corner.x);
      bottom = std::max(bottom, corner.y);
    }
    if (!std::isfinite(left) || !std::isfinite(top) || !std::isfinite(right) || !std::isfinite(bottom))
    {
      return {};
    }
    const auto x0 = static_cast<int32>(std::floor(left)) - 1;
    const auto y0 = static_cast<int32>(std::floor(top)) - 1;
    const auto x1 = static_cast<int32>(std::ceil(right)) + 1;
    const auto y1 = static_cast<int32>(std::ceil(bottom)) + 1;
    return Intersect({.X = x0,
                      .Y = y0,
                      .Width = static_cast<uint32>(std::max(x1 - x0, 0)),
                      .Height = static_cast<uint32>(std::max(y1 - y0, 0))},
                     _state.Clip);
  }

  bool Renderer2D::CopiesBackdrop(BlendMode mode) noexcept
  {
    return mode != BlendMode::Normal && mode != BlendMode::PlusLighter && mode != BlendMode::PlusDarker;
  }

  void Renderer2D::End() noexcept
  {
    krys_assert_msg(_commands != nullptr, "End without Begin.");
    krys_debug_assert_msg(_currentLayer == NoLayer, "BeginLayer without a matching EndLayer.");
    while (_currentLayer != NoLayer)
    {
      EndLayer();
    }
    krys_debug_assert_msg(_savedStates.empty(), "Save without a matching Restore.");
    auto &commands = *_commands;
    _commands = nullptr;

    // Children ended before their parents, so each layer's bounds are complete when it's finished.
    for (const auto index : _layerOrder)
    {
      FinishLayer(index);
    }

    // The backdrop is laid out as the largest pass that copies one.
    uint32 backdropWidth = 0u;
    uint32 backdropHeight = 0u;
    bool targetCopies = false;
    bool additive = false;
    for (const auto &batch : _batches)
    {
      additive = additive || batch.Blend == BlendMode::PlusLighter;
      if (!CopiesBackdrop(batch.Blend) || batch.Backdrop.IsEmpty())
      {
        continue;
      }
      uint32 width = _target.Width;
      uint32 height = _target.Height;
      if (batch.Layer != NoLayer)
      {
        const auto desc = _device->GetTextureDesc(_layers[batch.Layer].Texture);
        if (!desc)
        {
          continue;
        }
        width = desc->Width;
        height = desc->Height;
      }
      else
      {
        targetCopies = true;
      }
      backdropWidth = std::max(backdropWidth, width);
      backdropHeight = std::max(backdropHeight, height);
    }

    // A swapchain can't be copied from, so a frame that blends straight onto one is drawn into a texture,
    // which is then composited into the window.
    TextureHandle offscreen {};
    if (targetCopies && _target.Swapchain.IsValid() && _target.Width > 0u && _target.Height > 0u)
    {
      offscreen = AcquireLayerTexture(_target.Width, _target.Height);
    }
    if (offscreen.IsValid())
    {
      const auto desc = _device->GetTextureDesc(offscreen);
      const float width = static_cast<float>(desc->Width);
      const float height = static_cast<float>(desc->Height);
      _batches.push_back({.FirstInstance = static_cast<uint32>(_rects.size()),
                          .InstanceCount = 1u,
                          .Scissor = {.X = 0, .Y = 0, .Width = _target.Width, .Height = _target.Height},
                          .Chain = NoChain,
                          .Texture = offscreen,
                          .Sampler = _nearestSampler,
                          .Layer = WindowLayer,
                          .Composite = NoLayer,
                          .Blend = BlendMode::Normal,
                          .Backdrop = {}});
      _rects.push_back(
        {.Rect = {0.f, 0.f, static_cast<float>(_target.Width), static_cast<float>(_target.Height)},
         .Radii = {},
         .Linear = {1.f, 0.f, 0.f, 1.f},
         .Translation = {},
         .UV = {0.f, 0.f, static_cast<float>(_target.Width) / width,
                static_cast<float>(_target.Height) / height},
         .HoleRect = {},
         .HoleRadii = {},
         .Colour = {255u, 255u, 255u, 255u},
         .Flags = Textured});
    }

    FrameResources resources {};
    if (backdropWidth > 0u && backdropHeight > 0u)
    {
      resources.Backdrop = AcquireLayerTexture(backdropWidth, backdropHeight);
    }
    if (!_rects.empty() && _target.Width > 0u && _target.Height > 0u)
    {
      auto created = GetPipeline(_target.Format, false);
      auto createdAdditive =
        additive ? GetPipeline(_target.Format, true) : Expected<PipelineHandle> {PipelineHandle {}};
      const uint64 bytes = _rects.size() * sizeof(RectInstance);
      resources.Instances = _device->AllocateTransient(bytes, alignof(RectInstance));
      if (!created || !createdAdditive)
      {
        KRYS_LOG_ERROR("Render2D: {}", !created ? created.error() : createdAdditive.error());
      }
      else if (resources.Instances.IsValid())
      {
        resources.Pipeline = *created;
        resources.AdditivePipeline = *createdAdditive;
        if (std::ranges::any_of(
              _layers, [](const Layer &layer)
              { return layer.Filter && layer.Filter->Type != LayerFilter2D::Kind::ColourMatrix; }))
        {
          if (auto blur = GetBlurPipeline(_target.Format))
          {
            resources.BlurPipeline = *blur;
          }
          else
          {
            KRYS_LOG_ERROR("Render2D: {}", blur.error());
          }
        }
        std::memcpy(resources.Instances.Data.data(), _rects.data(), bytes);
      }
    }

    // Each layer's pass comes before the pass that composites it; the target's comes last.
    for (const auto index : _layerOrder)
    {
      const auto &layer = _layers[index];
      if (!layer.Texture.IsValid())
      {
        continue;
      }
      const auto desc = _device->GetTextureDesc(layer.Texture);
      RecordPass(
        commands,
        {.Attachment = {.Texture = layer.Texture, .Load = AttachmentLoadOp::Clear, .ClearColour = {}},
         .Width = desc->Width,
         .Height = desc->Height,
         .OriginX = layer.Bounds.X,
         .OriginY = layer.Bounds.Y,
         .Layer = index},
        resources);

      // A blur is two passes, across into a scratch texture and down: back into the layer's texture, or a
      // shadow's into its own.
      if (layer.Filter && layer.Filter->Type != LayerFilter2D::Kind::ColourMatrix)
      {
        const bool shadow = layer.Filter->Type == LayerFilter2D::Kind::DropShadow;
        const float sigma = (shadow ? layer.Filter->Shadow.BlurRadius : layer.Filter->BlurRadius) / 2.f;
        const auto destination = shadow ? layer.ShadowTexture : layer.Texture;
        const PixelRect2D region {.X = 0, .Y = 0, .Width = layer.Bounds.Width, .Height = layer.Bounds.Height};
        if (!destination.IsValid())
        {
          continue;
        }
        if (sigma <= 0.f)
        {
          // A sharp shadow is the layer's alpha as it is.
          if (shadow)
          {
            const TextureRegion copied {.X = 0u, .Y = 0u, .Width = region.Width, .Height = region.Height};
            commands.BlitTexture(layer.Texture, copied, destination, copied, SamplerFilter::Nearest);
          }
          continue;
        }
        const auto scratch = AcquireLayerTexture(region.Width, region.Height);
        if (!scratch.IsValid())
        {
          continue;
        }
        RecordBlur(commands, resources, layer.Texture, scratch, region, {1.f, 0.f}, sigma);
        RecordBlur(commands, resources, scratch, destination, region, {0.f, 1.f}, sigma);
      }
    }

    ColourAttachment attachment {.Texture = _target.Texture, .Swapchain = _target.Swapchain};
    attachment.Load = _target.ClearColour ? AttachmentLoadOp::Clear : AttachmentLoadOp::Load;
    if (_target.ClearColour)
    {
      attachment.ClearColour = *_target.ClearColour;
    }
    if (offscreen.IsValid())
    {
      // The offscreen target starts as the window would, or transparent when the window keeps its
      // contents, which then show through; the window is cleared to transparent under an exact copy.
      const auto window = attachment;
      attachment = {.Texture = offscreen,
                    .Load = AttachmentLoadOp::Clear,
                    .ClearColour = _target.ClearColour ? *_target.ClearColour : Array<float, 4> {}};
      RecordPass(commands,
                 {.Attachment = attachment,
                  .Width = _target.Width,
                  .Height = _target.Height,
                  .OriginX = 0,
                  .OriginY = 0,
                  .Layer = NoLayer},
                 resources);
      attachment = window;
      attachment.ClearColour = {};
      RecordPass(commands,
                 {.Attachment = attachment,
                  .Width = _target.Width,
                  .Height = _target.Height,
                  .OriginX = 0,
                  .OriginY = 0,
                  .Layer = WindowLayer},
                 resources);
    }
    else
    {
      RecordPass(commands,
                 {.Attachment = attachment,
                  .Width = _target.Width,
                  .Height = _target.Height,
                  .OriginX = 0,
                  .OriginY = 0,
                  .Layer = NoLayer},
                 resources);
    }

    _stats = {.Instances = static_cast<uint32>(_rects.size()),
              .Batches = static_cast<uint32>(_batches.size()),
              .MasksRasterised = _masks.Stats().Rasterised,
              .MaskCacheHits = _masks.Stats().Hits,
              .GlyphsRasterised = _glyphs.Stats().Rasterised,
              .GlyphCacheHits = _glyphs.Stats().Hits,
              .BackdropCopies = _backdropCopies};
    _savedStates.clear();
    _rects.clear();
    _batches.clear();
    _chains.clear();
    _layers.clear();
    _layerOrder.clear();
    _gradients.clear();
    _gradientStops.clear();
    _patterns.clear();
    _colourMatrices.clear();
  }

  void Renderer2D::FinishLayer(uint32 index) noexcept
  {
    auto &layer = _layers[index];
    // A blur or a shadow reaches past what's drawn, within the clip the layer began with.
    float sigma = 0.f;
    if (layer.Filter && layer.Filter->Type == LayerFilter2D::Kind::Blur)
    {
      sigma = layer.Filter->BlurRadius / 2.f;
    }
    else if (layer.Filter && layer.Filter->Type == LayerFilter2D::Kind::DropShadow)
    {
      sigma = layer.Filter->Shadow.BlurRadius / 2.f;
    }
    layer.Bounds = Intersect(Inflate(layer.Bounds, BlurReach(sigma)), layer.Clip);
    if (!layer.HasComposite || layer.Opacity <= 0.f || layer.Bounds.IsEmpty())
    {
      // Nothing to show: the composite, if any, stays an empty instance.
      return;
    }

    layer.Texture = AcquireLayerTexture(layer.Bounds.Width, layer.Bounds.Height);
    if (!layer.Texture.IsValid())
    {
      return;
    }
    const auto desc = _device->GetTextureDesc(layer.Texture);
    const float width = static_cast<float>(desc->Width);
    const float height = static_cast<float>(desc->Height);
    const auto &bounds = layer.Bounds;

    // The composite draws the layer's texels one to one, premultiplied, at the layer's opacity.
    auto &instance = _rects[layer.CompositeInstance];
    instance.Rect = {static_cast<float>(bounds.X), static_cast<float>(bounds.Y),
                     static_cast<float>(bounds.Width), static_cast<float>(bounds.Height)};
    instance.UV = {0.f, 0.f, static_cast<float>(bounds.Width) / width,
                   static_cast<float>(bounds.Height) / height};
    instance.Colour = {255u, 255u, 255u, static_cast<uint8>(layer.Opacity * 255.f + 0.5f)};
    instance.Flags = Textured | static_cast<uint32>(layer.Blend) << BlendModeShift;
    auto &batch = _batches[layer.CompositeBatch];
    batch.Texture = layer.Texture;
    batch.Sampler = _nearestSampler;
    if (CopiesBackdrop(layer.Blend))
    {
      batch.Backdrop = Intersect(bounds, batch.Scissor);
    }
    if (layer.Filter && layer.Filter->Type == LayerFilter2D::Kind::ColourMatrix)
    {
      // feColorMatrix's rows, as the shader's column-major matrix and offset.
      const auto &m = layer.Filter->Matrix;
      ColourMatrixUniforms uniforms {};
      for (uint32 row = 0u; row < 4u; ++row)
      {
        for (uint32 column = 0u; column < 4u; ++column)
        {
          uniforms.Matrix[column * 4u + row] = m[row * 5u + column];
        }
        uniforms.Offset[row] = m[row * 5u + 4u];
      }
      batch.ColourMatrix = static_cast<uint32>(_colourMatrices.size());
      _colourMatrices.push_back(uniforms);
      instance.Flags |= ColourMatrixFilter;
    }

    auto composited = bounds;
    if (layer.Filter && layer.Filter->Type == LayerFilter2D::Kind::DropShadow)
    {
      // The shadow is the layer's alpha, blurred, in the shadow's colour, moved by its offset.
      layer.ShadowTexture = AcquireLayerTexture(bounds.Width, bounds.Height);
      if (layer.ShadowTexture.IsValid())
      {
        const auto &shadow = layer.Filter->Shadow;
        auto &cast = _rects[layer.ShadowInstance];
        cast.Rect = {instance.Rect[0] + shadow.Offset[0], instance.Rect[1] + shadow.Offset[1],
                     instance.Rect[2], instance.Rect[3]};
        cast.UV = instance.UV;
        const auto colour = shadow.Colour.Resolved();
        cast.Colour = {colour.Red, colour.Green, colour.Blue,
                       static_cast<uint8>(static_cast<float>(colour.Alpha) * layer.Opacity + 0.5f)};
        cast.Flags = Textured | AlphaMask | static_cast<uint32>(layer.Blend) << BlendModeShift;
        auto &shadowBatch = _batches[layer.ShadowBatch];
        shadowBatch.Texture = layer.ShadowTexture;
        shadowBatch.Sampler = _linearSampler;
        const auto moved = ToPixelRect(cast.Rect[0], cast.Rect[1], cast.Rect[0] + cast.Rect[2] + 1.f,
                                       cast.Rect[1] + cast.Rect[3] + 1.f);
        if (CopiesBackdrop(layer.Blend))
        {
          shadowBatch.Backdrop = Intersect(moved, shadowBatch.Scissor);
        }
        composited = Union(composited, moved);
      }
    }

    if (layer.Parent != NoLayer)
    {
      auto &parent = _layers[layer.Parent];
      parent.Bounds = Union(parent.Bounds, composited);
    }
  }

  TextureHandle Renderer2D::AcquireLayerTexture(uint32 width, uint32 height) noexcept
  {
    // Any texture this frame hasn't used that's large enough, the smallest first; this frame's layer passes
    // all run before the passes that composite them, so none can be shared within it.
    const uint64 frame = _device->FrameNumber();
    LayerTexture *best = nullptr;
    for (auto &texture : _layerTextures)
    {
      if (texture.LastUsedFrame < frame && texture.Format == _target.Format && texture.Width >= width
          && texture.Height >= height
          && (best == nullptr || texture.Width * texture.Height < best->Width * best->Height))
      {
        best = &texture;
      }
    }
    if (best != nullptr)
    {
      best->LastUsedFrame = frame;
      return best->Texture;
    }

    // New textures round up to 64 pixels, so slightly different layers can reuse them.
    const uint32 roundedWidth = (width + 63u) / 64u * 64u;
    const uint32 roundedHeight = (height + 63u) / 64u * 64u;
    auto texture = _device->CreateTexture({.Format = _target.Format,
                                           .Width = roundedWidth,
                                           .Height = roundedHeight,
                                           .Usage = TextureUsage::RenderTarget | TextureUsage::Sampled,
                                           .DebugName = u8"Render2D.Layer"});
    krys_debug_assert_msg(texture.has_value(), "Couldn't create a layer texture.");
    if (!texture)
    {
      return {};
    }
    _layerTextures.push_back({.Texture = *texture,
                              .Width = roundedWidth,
                              .Height = roundedHeight,
                              .Format = _target.Format,
                              .LastUsedFrame = frame});
    return *texture;
  }

  void Renderer2D::RecordPass(CommandBuffer &commands, const Pass &pass,
                              const FrameResources &resources) noexcept
  {
    commands.BeginRenderPass(RenderPassDesc::SingleColour(pass.Attachment));

    // The viewport covers the whole attachment, so a texture's own size, not what's drawn of it, maps
    // pixels; pooled textures are rounded up, and a target may use only part of its texture.
    uint32 width = pass.Width;
    uint32 height = pass.Height;
    if (pass.Attachment.Texture.IsValid())
    {
      if (const auto desc = _device->GetTextureDesc(pass.Attachment.Texture))
      {
        width = desc->Width;
        height = desc->Height;
      }
    }
    if (!resources.Pipeline.IsValid() || width == 0u || height == 0u)
    {
      commands.EndRenderPass();
      return;
    }

    auto frame = _device->AllocateTransient(sizeof(FrameUniforms));
    if (!frame.IsValid())
    {
      commands.EndRenderPass();
      return;
    }
    // Layers have the target's format, so every pass dithers alike.
    const auto format = _target.Format;
    const bool eightBit = format == TextureFormat::R8Unorm || format == TextureFormat::RG8Unorm
                          || format == TextureFormat::RGBA8Unorm || format == TextureFormat::BGRA8Unorm;
    const FrameUniforms uniforms {
      .ViewportSize = {static_cast<float>(width), static_cast<float>(height)},
      .Origin = {static_cast<float>(pass.OriginX), static_cast<float>(pass.OriginY)},
      .Dither = eightBit ? 1.f / 255.f : 0.f,
      .Padding = {}};
    std::memcpy(frame.Data.data(), &uniforms, sizeof(uniforms));

    PipelineHandle boundPipeline {};
    TextureHandle boundTexture {};
    SamplerHandle boundSampler {};
    uint64 boundChain = ~0ull;
    uint32 boundGradient = NoGradient;
    uint32 boundPattern = NoPattern;
    // Binds what every batch draws with, at the pass's start and again whenever it resumes.
    auto bindFrame = [&]
    {
      commands.BindPipeline(resources.Pipeline);
      commands.BindUniformBuffer(0u, frame.Buffer, frame.Offset, sizeof(FrameUniforms));
      commands.BindVertexBuffer(0u, resources.Instances.Buffer, resources.Instances.Offset);
      // Unit 2 always has a texture: drivers check every declared sampler at each draw.
      commands.BindTexture(2u, resources.Backdrop.IsValid() ? resources.Backdrop : _whiteTexture,
                           _nearestSampler);
      // Unit 3 too, for patterns.
      commands.BindTexture(3u, _whiteTexture, _nearestSampler);
      boundPipeline = resources.Pipeline;
      boundTexture = {};
      boundSampler = {};
      boundChain = ~0ull;
      boundGradient = NoGradient;
      boundPattern = NoPattern;
    };
    bindFrame();

    const PixelRect2D viewport {.X = 0, .Y = 0, .Width = width, .Height = height};
    for (const auto &batch : _batches)
    {
      if (batch.Layer != pass.Layer)
      {
        continue;
      }
      // The scissor is in target pixels; the pass's pixels start at its origin.
      const auto scissor = Intersect({.X = batch.Scissor.X - pass.OriginX,
                                      .Y = batch.Scissor.Y - pass.OriginY,
                                      .Width = batch.Scissor.Width,
                                      .Height = batch.Scissor.Height},
                                     viewport);
      if (scissor.IsEmpty())
      {
        continue;
      }

      // What's beneath the batch is copied first: the pass ends, the pixels the batch reaches are copied
      // into the backdrop at the same place, and the pass resumes with its contents kept.
      if (CopiesBackdrop(batch.Blend))
      {
        const auto region = Intersect({.X = batch.Backdrop.X - pass.OriginX,
                                       .Y = batch.Backdrop.Y - pass.OriginY,
                                       .Width = batch.Backdrop.Width,
                                       .Height = batch.Backdrop.Height},
                                      scissor);
        if (region.IsEmpty() || !resources.Backdrop.IsValid() || !pass.Attachment.Texture.IsValid())
        {
          continue;
        }
        commands.EndRenderPass();
        const TextureRegion copied {.X = static_cast<uint32>(region.X),
                                    .Y = static_cast<uint32>(region.Y),
                                    .Width = region.Width,
                                    .Height = region.Height};
        commands.BlitTexture(pass.Attachment.Texture, copied, resources.Backdrop, copied,
                             SamplerFilter::Nearest);
        ++_backdropCopies;
        auto resumed = pass.Attachment;
        resumed.Load = AttachmentLoadOp::Load;
        commands.BeginRenderPass(RenderPassDesc::SingleColour(resumed));
        bindFrame();
      }

      const auto pipeline =
        batch.Blend == BlendMode::PlusLighter ? resources.AdditivePipeline : resources.Pipeline;
      if (pipeline != boundPipeline)
      {
        commands.BindPipeline(pipeline);
        boundPipeline = pipeline;
      }

      // Every batch binds its clip chain, even an empty one: the shader always reads the block.
      if (batch.Chain != boundChain)
      {
        auto clips = _device->AllocateTransient(sizeof(ClipUniforms));
        if (!clips.IsValid())
        {
          break;
        }
        ClipUniforms data {};
        TextureHandle clipMask = _whiteTexture;
        if (batch.Chain != NoChain)
        {
          const auto &chain = _chains[batch.Chain];
          if (chain.Mask.Texture.IsValid())
          {
            const auto &bounds = chain.Mask.Bounds;
            data.MaskMode = chain.MaskOut ? ClipOutMask : ClipToMask;
            data.MaskBounds = {bounds.X, bounds.Y, static_cast<int32>(bounds.Width),
                               static_cast<int32>(bounds.Height)};
            data.MaskOrigin = {chain.Mask.Texels.X, chain.Mask.Texels.Y, 0, 0};
            clipMask = chain.Mask.Texture;
          }
          const auto &shapes = chain.Shapes;
          data.Count = static_cast<uint32>(shapes.size());
          for (size_t i = 0u; i < shapes.size(); ++i)
          {
            const auto &clip = shapes[i];
            const auto &rect = clip.Shape.Rect;
            const auto &radii = clip.Shape.Radii;
            const auto &inverse = clip.Inverse;
            data.Clips[i] = {.Rect = {rect.X, rect.Y, rect.Width, rect.Height},
                             .Radii = {radii.TopLeft, radii.TopRight, radii.BottomRight, radii.BottomLeft},
                             .InverseLinear = {inverse.A, inverse.B, inverse.C, inverse.D},
                             .InverseTranslation = {inverse.E, inverse.F, clip.Out ? 1.f : 0.f, 0.f}};
          }
        }
        std::memcpy(clips.Data.data(), &data, sizeof(data));
        commands.BindUniformBuffer(1u, clips.Buffer, clips.Offset, sizeof(ClipUniforms));
        // Unit 1 always has a texture: drivers check every declared sampler at each draw.
        commands.BindTexture(1u, clipMask, _nearestSampler);
        boundChain = batch.Chain;
      }

      // Only gradient instances read the gradient's block.
      if (batch.Gradient != NoGradient && batch.Gradient != boundGradient)
      {
        auto block = _device->AllocateTransient(sizeof(GradientUniforms));
        if (!block.IsValid())
        {
          break;
        }
        const auto &gradient = _gradients[batch.Gradient];
        const auto &inverse = gradient.Inverse;
        GradientUniforms data {};
        data.Shape = gradient.Shape;
        data.Spread = static_cast<uint32>(gradient.Spread);
        data.StopCount = gradient.StopCount;
        data.InverseLinear = {inverse.A, inverse.B, inverse.C, inverse.D};
        data.InverseTranslation = {inverse.E, inverse.F, 0.f, 0.f};
        data.Radial = gradient.Radial;
        for (uint32 i = 0u; i < gradient.StopCount; ++i)
        {
          const auto &stop = _gradientStops[gradient.FirstStop + i];
          data.Offsets[i] = stop.Offset;
          data.Colours[i] = stop.Colour;
        }
        std::memcpy(block.Data.data(), &data, sizeof(data));
        commands.BindUniformBuffer(2u, block.Buffer, block.Offset, sizeof(GradientUniforms));
        boundGradient = batch.Gradient;
      }

      // Likewise, only pattern instances read the pattern's block and texture.
      if (batch.Pattern != NoPattern && batch.Pattern != boundPattern)
      {
        auto block = _device->AllocateTransient(sizeof(PatternUniforms));
        if (!block.IsValid())
        {
          break;
        }
        const auto &pattern = _patterns[batch.Pattern];
        const auto &inverse = pattern.Inverse;
        const PatternUniforms data {.InverseLinear = {inverse.A, inverse.B, inverse.C, inverse.D},
                                    .InverseTranslation = {inverse.E, inverse.F, 0.f, 0.f},
                                    .TileAndSpacing = pattern.TileAndSpacing,
                                    .Flags = {pattern.Flags, 0u, 0u, 0u}};
        std::memcpy(block.Data.data(), &data, sizeof(data));
        commands.BindUniformBuffer(3u, block.Buffer, block.Offset, sizeof(PatternUniforms));
        commands.BindTexture(3u, pattern.Texture, pattern.Sampler);
        boundPattern = batch.Pattern;
      }

      // Only filtered composites read a colour matrix.
      if (batch.ColourMatrix != NoColourMatrix)
      {
        auto block = _device->AllocateTransient(sizeof(ColourMatrixUniforms));
        if (!block.IsValid())
        {
          break;
        }
        std::memcpy(block.Data.data(), &_colourMatrices[batch.ColourMatrix], sizeof(ColourMatrixUniforms));
        commands.BindUniformBuffer(4u, block.Buffer, block.Offset, sizeof(ColourMatrixUniforms));
      }

      // Untextured batches never sample, but drivers still check the sampler's texture at every draw, so
      // they get the white texture unless another one is already bound.
      const auto texture = batch.Texture.IsValid()  ? batch.Texture
                           : boundTexture.IsValid() ? boundTexture
                                                    : _whiteTexture;
      const auto sampler = batch.Texture.IsValid()  ? batch.Sampler
                           : boundSampler.IsValid() ? boundSampler
                                                    : _nearestSampler;
      if (texture != boundTexture || sampler != boundSampler)
      {
        commands.BindTexture(0u, texture, sampler);
        boundTexture = texture;
        boundSampler = sampler;
      }
      commands.SetScissor(scissor.X, scissor.Y, scissor.Width, scissor.Height);
      commands.Draw(4u, batch.InstanceCount, 0u, batch.FirstInstance);
    }
    commands.EndRenderPass();
  }

  Expected<PipelineHandle> Renderer2D::GetPipeline(TextureFormat format, bool additive) noexcept
  {
    for (const auto &entry : _pipelines)
    {
      if (entry.Format == format && entry.Additive == additive)
      {
        return entry.Pipeline;
      }
    }

    // Additive pipelines draw `BlendMode::PlusLighter`: the sum of the premultiplied colours, clamped.
    const auto blend = additive ? BlendState {.Enabled = true,
                                              .SourceColour = BlendFactor::One,
                                              .DestinationColour = BlendFactor::One,
                                              .SourceAlpha = BlendFactor::One,
                                              .DestinationAlpha = BlendFactor::One}
                                : BlendState::PremultipliedAlpha();
    PipelineDesc desc {
      .VertexShader = _vertexShader,
      .FragmentShader = _fragmentShader,
      .Topology = PrimitiveTopology::TriangleStrip,
      .Blend = blend,
      .Raster = {.ScissorTest = true},
      .ColourFormats = {format},
      .DebugName = additive ? u8"Render2D.Rect.Additive" : u8"Render2D.Rect",
    };
    desc.VertexBuffers.push_back({
      .Stride = sizeof(RectInstance),
      .InputRate = VertexInputRate::PerInstance,
      .Attributes = {{0u, VertexFormat::Float4, offsetof(RectInstance, Rect)},
                     {1u, VertexFormat::Float4, offsetof(RectInstance, Radii)},
                     {2u, VertexFormat::Float4, offsetof(RectInstance, Linear)},
                     {3u, VertexFormat::Float2, offsetof(RectInstance, Translation)},
                     {4u, VertexFormat::Float4, offsetof(RectInstance, UV)},
                     {5u, VertexFormat::UByte4Norm, offsetof(RectInstance, Colour)},
                     {6u, VertexFormat::UInt, offsetof(RectInstance, Flags)},
                     {7u, VertexFormat::Float4, offsetof(RectInstance, HoleRect)},
                     {8u, VertexFormat::Float4, offsetof(RectInstance, HoleRadii)}},
    });

    auto pipeline = _device->CreatePipeline(desc);
    if (!pipeline)
    {
      return Unexpected<string>(pipeline.error());
    }
    _pipelines.push_back({.Format = format, .Additive = additive, .Pipeline = *pipeline});
    return *pipeline;
  }

  Expected<PipelineHandle> Renderer2D::GetBlurPipeline(TextureFormat format) noexcept
  {
    for (const auto &entry : _blurPipelines)
    {
      if (entry.Format == format)
      {
        return entry.Pipeline;
      }
    }

    // The blur replaces what's in the destination: no blending, and the quad covers the region.
    const PipelineDesc desc {
      .VertexShader = _blurVertexShader,
      .FragmentShader = _blurFragmentShader,
      .Topology = PrimitiveTopology::TriangleStrip,
      .Blend = {},
      .Raster = {.ScissorTest = false},
      .ColourFormats = {format},
      .DebugName = u8"Render2D.Blur",
    };
    auto pipeline = _device->CreatePipeline(desc);
    if (!pipeline)
    {
      return Unexpected<string>(pipeline.error());
    }
    _blurPipelines.push_back({.Format = format, .Additive = false, .Pipeline = *pipeline});
    return *pipeline;
  }

  void Renderer2D::RecordBlur(CommandBuffer &commands, const FrameResources &resources, TextureHandle source,
                              TextureHandle destination, const PixelRect2D &region, Array<float, 2> direction,
                              float sigma) noexcept
  {
    const auto desc = _device->GetTextureDesc(destination);
    if (!resources.BlurPipeline.IsValid() || !desc || region.IsEmpty())
    {
      return;
    }
    auto block = _device->AllocateTransient(sizeof(BlurUniforms));
    if (!block.IsValid())
    {
      return;
    }
    const BlurUniforms uniforms {
      .Region = {static_cast<float>(region.X), static_cast<float>(region.Y), static_cast<float>(region.Width),
                 static_cast<float>(region.Height)},
      .TextureSize = {static_cast<float>(desc->Width), static_cast<float>(desc->Height)},
      .Direction = direction,
      .Sigma = sigma,
      .Reach = BlurReach(sigma),
      .Padding = {}};
    std::memcpy(block.Data.data(), &uniforms, sizeof(uniforms));

    commands.BeginRenderPass(RenderPassDesc::SingleColour(
      {.Texture = destination, .Load = AttachmentLoadOp::Clear, .ClearColour = {}}));
    commands.SetViewport(0.f, 0.f, static_cast<float>(desc->Width), static_cast<float>(desc->Height));
    commands.BindPipeline(resources.BlurPipeline);
    commands.BindUniformBuffer(0u, block.Buffer, block.Offset, sizeof(BlurUniforms));
    commands.BindTexture(0u, source, _nearestSampler);
    commands.Draw(4u);
    commands.EndRenderPass();
  }

  PixelRect2D Renderer2D::ToPixelRect(float left, float top, float right, float bottom) noexcept
  {
    const auto x0 = static_cast<int32>(std::ceil(left - 0.5f));
    const auto y0 = static_cast<int32>(std::ceil(top - 0.5f));
    const auto x1 = static_cast<int32>(std::ceil(right - 0.5f));
    const auto y1 = static_cast<int32>(std::ceil(bottom - 0.5f));
    return {.X = x0,
            .Y = y0,
            .Width = x1 > x0 ? static_cast<uint32>(x1 - x0) : 0u,
            .Height = y1 > y0 ? static_cast<uint32>(y1 - y0) : 0u};
  }

  PixelRect2D Renderer2D::Union(const PixelRect2D &a, const PixelRect2D &b) noexcept
  {
    if (a.IsEmpty())
    {
      return b;
    }
    if (b.IsEmpty())
    {
      return a;
    }
    const int32 x0 = std::min(a.X, b.X);
    const int32 y0 = std::min(a.Y, b.Y);
    const int32 x1 = std::max(a.X + static_cast<int32>(a.Width), b.X + static_cast<int32>(b.Width));
    const int32 y1 = std::max(a.Y + static_cast<int32>(a.Height), b.Y + static_cast<int32>(b.Height));
    return {.X = x0, .Y = y0, .Width = static_cast<uint32>(x1 - x0), .Height = static_cast<uint32>(y1 - y0)};
  }

  PixelRect2D Renderer2D::Intersect(const PixelRect2D &a, const PixelRect2D &b) noexcept
  {
    const int32 x0 = std::max(a.X, b.X);
    const int32 y0 = std::max(a.Y, b.Y);
    const int32 x1 = std::min(a.X + static_cast<int32>(a.Width), b.X + static_cast<int32>(b.Width));
    const int32 y1 = std::min(a.Y + static_cast<int32>(a.Height), b.Y + static_cast<int32>(b.Height));
    return {.X = x0,
            .Y = y0,
            .Width = x1 > x0 ? static_cast<uint32>(x1 - x0) : 0u,
            .Height = y1 > y0 ? static_cast<uint32>(y1 - y0) : 0u};
  }
}
