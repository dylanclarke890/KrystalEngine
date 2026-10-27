#include "Krystal.Gfx/Render2D/FilterRenderer2D.hpp"
#include "Krystal.Core/Log/ILogger.hpp"
#include <algorithm>
#include <cmath>
#include <cstring>

namespace krys::gfx
{
  namespace
  {
    constexpr const char8_t *FilterVertexSource = u8R"(#version 450
layout(binding = 0, std140) uniform Filter
{
  ivec4 u_Region;
  ivec4 u_Input0;
  ivec4 u_Input1;
  ivec4 u_Bounds;
  uvec4 u_Mode;
  uvec4 u_Convert;
  vec4 u_TextureSize;
  vec4 u_P0;
  vec4 u_P1;
  vec4 u_P2;
  vec4 u_P3;
  vec4 u_Matrix[5];
  vec4 u_Table[256];
};

layout(location = 0) out vec2 v_Texel;

void main()
{
  vec2 corner = vec2(gl_VertexID & 1, gl_VertexID >> 1);
  vec2 texel = corner * vec2(u_Region.zw);
  vec2 ndc = texel / u_TextureSize.xy * 2.0 - 1.0;
  gl_Position = vec4(ndc.x, -ndc.y, 0.0, 1.0);
  v_Texel = texel;
}
)";

    // One node of a filter graph, or one pass of a blur or morphology. Positions are in the graph's pixels:
    // the node writes u_Region, and each input holds its own region. Colours are premultiplied.
    constexpr const char8_t *FilterFragmentSource = u8R"(#version 450
layout(binding = 0, std140) uniform Filter
{
  ivec4 u_Region;      // the pixels written: x, y, width, height
  ivec4 u_Input0;      // the pixels each input holds
  ivec4 u_Input1;
  ivec4 u_Bounds;      // the region edge modes keep to, or a tile's rect
  uvec4 u_Mode;        // the kind, two options, and the pass of a two-pass kind
  uvec4 u_Convert;     // how each input is read, then the edge mode
  vec4 u_TextureSize;
  vec4 u_P0;
  vec4 u_P1;
  vec4 u_P2;
  vec4 u_P3;
  vec4 u_Matrix[5];
  vec4 u_Table[256];   // a lookup table, or a kernel four values to an entry
};

layout(binding = 0) uniform sampler2D u_Source0;
layout(binding = 1) uniform sampler2D u_Source1;
layout(binding = 2) uniform sampler2D u_Turbulence;

layout(location = 0) in vec2 v_Texel;
layout(location = 0) out vec4 o_Colour;

const uint KindColourMatrix = 0u;
const uint KindComponentTransfer = 1u;
const uint KindOffset = 2u;
const uint KindFlood = 3u;
const uint KindBlend = 4u;
const uint KindComposite = 5u;
const uint KindConvolveMatrix = 6u;
const uint KindMorphology = 7u;
const uint KindBlur = 8u;
const uint KindDisplacementMap = 9u;
const uint KindDiffuseLighting = 10u;
const uint KindSpecularLighting = 11u;
const uint KindTurbulence = 12u;
const uint KindTile = 13u;

// How an input is read.
const uint ReadStraight = 1u;
const uint ReadToLinear = 2u;
const uint ReadToSrgb = 4u;

const uint EdgeDuplicate = 1u;
const uint EdgeWrap = 2u;

const uint OperatorOver = 0u;
const uint OperatorIn = 1u;
const uint OperatorOut = 2u;
const uint OperatorAtop = 3u;
const uint OperatorXor = 4u;
const uint OperatorLighter = 5u;
const uint OperatorArithmetic = 6u;

const uint LightPoint = 1u;
const uint LightSpot = 2u;

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

vec3 ToLinear(vec3 c)
{
  return mix(c / 12.92, pow((c + 0.055) / 1.055, vec3(2.4)), greaterThan(c, vec3(0.04045)));
}

vec3 ToSrgb(vec3 c)
{
  return mix(c * 12.92, 1.055 * pow(c, vec3(1.0 / 2.4)) - 0.055, greaterThan(c, vec3(0.0031308)));
}

vec4 Unpremultiply(vec4 c)
{
  return c.a > 0.0 ? vec4(clamp(c.rgb / c.a, 0.0, 1.0), c.a) : vec4(0.0);
}

vec4 Premultiply(vec4 c)
{
  return vec4(c.rgb * c.a, c.a);
}

vec4 Convert(vec4 c, uint read)
{
  if ((read & ReadStraight) != 0u)
  {
    c = Premultiply(c);
  }
  if ((read & (ReadToLinear | ReadToSrgb)) != 0u && c.a > 0.0)
  {
    vec3 s = clamp(c.rgb / c.a, 0.0, 1.0);
    s = (read & ReadToLinear) != 0u ? ToLinear(s) : ToSrgb(s);
    c.rgb = s * c.a;
  }
  return c;
}

// An input's pixel, premultiplied in the node's colour space; transparent outside its region.
vec4 Fetch(uint index, ivec2 p)
{
  ivec4 region = index == 0u ? u_Input0 : u_Input1;
  ivec2 t = p - region.xy;
  if (any(lessThan(t, ivec2(0))) || any(greaterThanEqual(t, region.zw)))
  {
    return vec4(0.0);
  }
  vec4 c = index == 0u ? texelFetch(u_Source0, t, 0) : texelFetch(u_Source1, t, 0);
  return Convert(c, index == 0u ? u_Convert.x : u_Convert.y);
}

// Where p reads under the edge mode, kept to u_Bounds; false when it reads nothing.
bool EdgeAt(inout ivec2 p, uint mode)
{
  ivec2 low = u_Bounds.xy;
  ivec2 high = low + u_Bounds.zw;
  if (all(greaterThanEqual(p, low)) && all(lessThan(p, high)))
  {
    return true;
  }
  if (mode == EdgeDuplicate)
  {
    p = clamp(p, low, high - 1);
    return true;
  }
  if (mode == EdgeWrap)
  {
    ivec2 d = p - low;
    d -= u_Bounds.zw * ivec2(floor(vec2(d) / vec2(u_Bounds.zw)));
    p = low + d;
    return true;
  }
  return false;
}

bool Inside(ivec2 p, ivec4 rect)
{
  return all(greaterThanEqual(p, rect.xy)) && all(lessThan(p, rect.xy + rect.zw));
}

float KernelAt(int i)
{
  return u_Table[i >> 2][i & 3];
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

vec4 ColourMatrix(vec4 c)
{
  vec4 s = Unpremultiply(c);
  vec4 r = vec4(dot(u_Matrix[0], s), dot(u_Matrix[1], s), dot(u_Matrix[2], s), dot(u_Matrix[3], s)) + u_Matrix[4];
  return Premultiply(clamp(r, 0.0, 1.0));
}

vec4 ComponentTransfer(vec4 c)
{
  ivec4 i = ivec4(floor(Unpremultiply(c) * 255.0 + 0.5));
  vec4 r = vec4(u_Table[i.r].r, u_Table[i.g].g, u_Table[i.b].b, u_Table[i.a].a);
  return Premultiply(r);
}

vec4 Offset(ivec2 p)
{
  // Bilinear, so fractional offsets move by part of a pixel.
  vec2 at = vec2(p) - u_P0.xy;
  vec2 base = floor(at);
  vec2 f = at - base;
  ivec2 b = ivec2(base);
  vec4 top = mix(Fetch(0u, b), Fetch(0u, b + ivec2(1, 0)), f.x);
  vec4 bottom = mix(Fetch(0u, b + ivec2(0, 1)), Fetch(0u, b + ivec2(1, 1)), f.x);
  return mix(top, bottom, f.y);
}

vec4 BlendInputs(ivec2 p)
{
  vec4 s = Fetch(0u, p);
  vec4 b = Fetch(1u, p);
  vec3 sc = s.a > 0.0 ? clamp(s.rgb / s.a, 0.0, 1.0) : vec3(0.0);
  vec3 bc = b.a > 0.0 ? clamp(b.rgb / b.a, 0.0, 1.0) : vec3(0.0);
  vec3 mixed = clamp(Blend(u_Mode.y, bc, sc), 0.0, 1.0);
  return vec4(s.rgb * (1.0 - b.a) + b.rgb * (1.0 - s.a) + s.a * b.a * mixed, s.a + b.a - s.a * b.a);
}

vec4 Composite(ivec2 p)
{
  vec4 s = Fetch(0u, p);
  vec4 d = Fetch(1u, p);
  switch (u_Mode.y)
  {
    case OperatorIn: return s * d.a;
    case OperatorOut: return s * (1.0 - d.a);
    case OperatorAtop: return s * d.a + d * (1.0 - s.a);
    case OperatorXor: return s * (1.0 - d.a) + d * (1.0 - s.a);
    case OperatorLighter: return min(s + d, vec4(1.0));
    case OperatorArithmetic:
    {
      vec4 r = clamp(u_P0.x * s * d + u_P0.y * s + u_P0.z * d + u_P0.w, 0.0, 1.0);
      return vec4(min(r.rgb, vec3(r.a)), r.a);
    }
    default: return s + d * (1.0 - s.a);
  }
}

vec4 ConvolveMatrix(ivec2 p)
{
  int columns = int(u_Mode.y);
  int rows = int(u_Mode.z);
  ivec2 target = ivec2(u_P1.xy);
  bool preserveAlpha = u_P0.z > 0.5;
  int count = columns * rows;
  vec4 total = vec4(0.0);
  for (int j = 0; j < rows; ++j)
  {
    for (int i = 0; i < columns; ++i)
    {
      ivec2 q = p + ivec2(i, j) - target;
      if (!EdgeAt(q, u_Convert.z))
      {
        continue;
      }
      vec4 c = Fetch(0u, q);
      if (preserveAlpha)
      {
        c = Unpremultiply(c);
      }
      // The kernel is applied turned half way round, as a convolution is.
      total += KernelAt(count - 1 - (j * columns + i)) * c;
    }
  }
  vec4 r = total / u_P0.x + u_P0.y;
  if (preserveAlpha)
  {
    float alpha = Unpremultiply(Fetch(0u, p)).a;
    return Premultiply(vec4(clamp(r.rgb, 0.0, 1.0), alpha));
  }
  float alpha = clamp(r.a, 0.0, 1.0);
  return vec4(clamp(r.rgb, vec3(0.0), vec3(alpha)), alpha);
}

vec4 Morphology(ivec2 p)
{
  ivec2 direction = u_Mode.w == 0u ? ivec2(1, 0) : ivec2(0, 1);
  int radius = int(u_Mode.w == 0u ? u_P0.x : u_P0.y);
  bool dilate = u_Mode.y != 0u;
  vec4 extremum = dilate ? vec4(0.0) : vec4(1.0);
  bool any = false;
  for (int i = -radius; i <= radius; ++i)
  {
    ivec2 q = p + direction * i;
    if (!Inside(q, u_Bounds))
    {
      continue;
    }
    vec4 c = Fetch(0u, q);
    extremum = dilate ? max(extremum, c) : min(extremum, c);
    any = true;
  }
  return any ? extremum : vec4(0.0);
}

vec4 BlurPass(ivec2 p)
{
  ivec2 direction = u_Mode.w == 0u ? ivec2(1, 0) : ivec2(0, 1);
  float sigma = u_Mode.w == 0u ? u_P0.x : u_P0.y;
  if (sigma <= 0.0)
  {
    ivec2 q = p;
    return EdgeAt(q, u_Convert.z) ? Fetch(0u, q) : vec4(0.0);
  }
  int reach = min(int(ceil(3.0 * sigma)), 256);
  float k = -0.5 / (sigma * sigma);
  vec4 sum = vec4(0.0);
  float total = 0.0;
  for (int i = -reach; i <= reach; ++i)
  {
    float weight = exp(float(i * i) * k);
    total += weight;
    ivec2 q = p + direction * i;
    if (EdgeAt(q, u_Convert.z))
    {
      sum += weight * Fetch(0u, q);
    }
  }
  return sum / total;
}

vec4 DisplacementMap(ivec2 p)
{
  vec4 map = Unpremultiply(Fetch(1u, p));
  float x = map[u_Mode.y] * u_P0.x + 0.5 - 0.5 * u_P0.x;
  float y = map[u_Mode.z] * u_P0.y + 0.5 - 0.5 * u_P0.y;
  ivec2 q = p + ivec2(int(x), int(y));
  return Inside(q, u_Bounds) ? Fetch(0u, q) : vec4(0.0);
}

float AlphaAt(ivec2 p, int dx, int dy)
{
  return Fetch(0u, p + ivec2(dx, dy)).a;
}

vec4 Lighting(ivec2 p)
{
  int width = u_Region.z;
  int height = u_Region.w;
  if (width <= 2 || height <= 2)
  {
    return Fetch(0u, p);
  }
  ivec2 local = p - u_Region.xy;
  bool left = local.x == 0;
  bool right = local.x == width - 1;
  bool top = local.y == 0;
  bool bottom = local.y == height - 1;

  float tl = AlphaAt(p, -1, -1), t = AlphaAt(p, 0, -1), tr = AlphaAt(p, 1, -1);
  float l = AlphaAt(p, -1, 0), c = AlphaAt(p, 0, 0), r = AlphaAt(p, 1, 0);
  float bl = AlphaAt(p, -1, 1), b = AlphaAt(p, 0, 1), br = AlphaAt(p, 1, 1);

  // The Sobel kernels, over the neighbours inside the region, with the spec's factor for each case.
  vec2 normal;
  vec2 factor;
  if (top && left)
  {
    normal = vec2(-2.0 * c + 2.0 * r - b + br, -2.0 * c - r + 2.0 * b + br);
    factor = vec2(-2.0 / 3.0);
  }
  else if (top && right)
  {
    normal = vec2(-2.0 * l + 2.0 * c - bl + b, -l - 2.0 * c + bl + 2.0 * b);
    factor = vec2(-2.0 / 3.0);
  }
  else if (bottom && left)
  {
    normal = vec2(-t + tr - 2.0 * c + 2.0 * r, -2.0 * t - tr + 2.0 * c + r);
    factor = vec2(-2.0 / 3.0);
  }
  else if (bottom && right)
  {
    normal = vec2(-tl + t - 2.0 * l + 2.0 * c, -tl - 2.0 * t + l + 2.0 * c);
    factor = vec2(-2.0 / 3.0);
  }
  else if (top)
  {
    normal = vec2(-2.0 * l + 2.0 * r - bl + br, -l - 2.0 * c - r + bl + 2.0 * b + br);
    factor = vec2(-1.0 / 3.0, -1.0 / 2.0);
  }
  else if (bottom)
  {
    normal = vec2(-tl + tr - 2.0 * l + 2.0 * r, -tl - 2.0 * t - tr + l + 2.0 * c + r);
    factor = vec2(-1.0 / 3.0, -1.0 / 2.0);
  }
  else if (left)
  {
    normal = vec2(-t + tr - 2.0 * c + 2.0 * r - b + br, -2.0 * t - tr + 2.0 * b + br);
    factor = vec2(-1.0 / 2.0, -1.0 / 3.0);
  }
  else if (right)
  {
    normal = vec2(-tl + t - 2.0 * l + 2.0 * c - bl + b, -tl - 2.0 * t + bl + 2.0 * b);
    factor = vec2(-1.0 / 2.0, -1.0 / 3.0);
  }
  else
  {
    normal = vec2(-tl + tr - 2.0 * l + 2.0 * r - bl + br, -tl - 2.0 * t - tr + bl + 2.0 * b + br);
    factor = vec2(-1.0 / 4.0);
  }

  float surfaceScale = u_P0.x;
  vec3 position = vec3(vec2(local), c * surfaceScale);
  vec3 lightVector = u_P2.xyz;
  vec3 colour = u_P1.rgb;
  if (u_Mode.y == LightPoint || u_Mode.y == LightSpot)
  {
    lightVector = u_P2.xyz - position;
  }
  float lightLength = length(lightVector);
  if (u_Mode.y == LightSpot)
  {
    float cosine = dot(lightVector, u_P3.xyz) / lightLength;
    float cutOff = u_Matrix[0].x;
    float fullLight = u_Matrix[0].y;
    if (cosine > cutOff)
    {
      colour = vec3(0.0);
    }
    else
    {
      float strength = u_P3.w == 1.0 ? -cosine : pow(-cosine, u_P3.w);
      if (cosine > fullLight)
      {
        strength *= (cutOff - cosine) / (cutOff - fullLight);
      }
      colour *= min(strength, 1.0);
    }
  }

  vec3 n = vec3(factor * normal * surfaceScale, 1.0);
  float strength;
  if (u_Mode.x == KindDiffuseLighting)
  {
    strength = u_P0.y * dot(n, lightVector) / (length(n) * lightLength);
  }
  else
  {
    vec3 halfway = vec3(lightVector.xy, lightVector.z + lightLength);
    float cosine = dot(n, halfway) / (length(n) * length(halfway));
    strength = u_P0.z * (u_P0.w == 1.0 ? cosine : pow(cosine, u_P0.w));
  }
  vec3 lit = clamp(strength, 0.0, 1.0) * colour;
  if (u_Mode.x == KindDiffuseLighting)
  {
    return vec4(lit, 1.0);
  }
  return vec4(lit, max(lit.r, max(lit.g, lit.b)));
}

const int BlockSize = 256;
const int BlockMask = 255;
const float PerlinNoise = 4096.0;

int Lattice(int i)
{
  return int(texelFetch(u_Turbulence, ivec2(i, 0), 0).r);
}

vec2 Gradient(int channel, int i)
{
  vec4 texel = texelFetch(u_Turbulence, ivec2(i, 1 + channel / 2), 0);
  return (channel & 1) == 0 ? texel.xy : texel.zw;
}

float SCurve(float t)
{
  return t * t * (3.0 - 2.0 * t);
}

// The noise function of SVG 1.1's feTurbulence reference code; stitch is the width, height, x wrap and y
// wrap.
vec4 Noise2D(vec2 v, ivec4 stitch, bool stitching)
{
  float tx = v.x + PerlinNoise;
  int bx0 = int(tx);
  int bx1 = bx0 + 1;
  float rx0 = tx - float(bx0);
  float ty = v.y + PerlinNoise;
  int by0 = int(ty);
  int by1 = by0 + 1;
  float ry0 = ty - float(by0);
  if (stitching)
  {
    bx0 -= bx0 >= stitch.z ? stitch.x : 0;
    bx1 -= bx1 >= stitch.z ? stitch.x : 0;
    by0 -= by0 >= stitch.w ? stitch.y : 0;
    by1 -= by1 >= stitch.w ? stitch.y : 0;
  }
  bx0 &= BlockMask;
  bx1 &= BlockMask;
  by0 &= BlockMask;
  by1 &= BlockMask;
  int i = Lattice(bx0);
  int j = Lattice(bx1);
  float sx = SCurve(rx0);
  float sy = SCurve(ry0);
  vec4 result;
  for (int channel = 0; channel < 4; ++channel)
  {
    vec2 q = Gradient(channel, Lattice(i + by0));
    float u = rx0 * q.x + ry0 * q.y;
    q = Gradient(channel, Lattice(j + by0));
    float v = (rx0 - 1.0) * q.x + ry0 * q.y;
    float a = mix(u, v, sx);
    q = Gradient(channel, Lattice(i + by1));
    u = rx0 * q.x + (ry0 - 1.0) * q.y;
    q = Gradient(channel, Lattice(j + by1));
    v = (rx0 - 1.0) * q.x + (ry0 - 1.0) * q.y;
    float b = mix(u, v, sx);
    result[channel] = mix(a, b, sy);
  }
  return result;
}

vec4 Turbulence(ivec2 p)
{
  bool fractalNoise = u_Mode.y != 0u;
  int octaves = int(u_Mode.z);
  bool stitching = u_P2.x > 0.5;
  ivec4 stitch = ivec4(u_P1);
  // Each pixel samples at its bottom-right corner, as the reference code does.
  vec2 point = (vec2(p) + 1.0) / u_P0.zw;
  vec2 noise = point * u_P0.xy;
  float ratio = 1.0;
  vec4 sum = vec4(0.0);
  for (int octave = 0; octave < octaves; ++octave)
  {
    vec4 n = Noise2D(noise, stitch, stitching);
    sum += (fractalNoise ? n : abs(n)) / ratio;
    noise *= 2.0;
    ratio *= 2.0;
    if (stitching)
    {
      stitch.x *= 2;
      stitch.z = 2 * stitch.z - int(PerlinNoise);
      stitch.y *= 2;
      stitch.w = 2 * stitch.w - int(PerlinNoise);
    }
  }
  if (fractalNoise)
  {
    sum = sum * 0.5 + 0.5;
  }
  vec4 colour = clamp(trunc(sum * 255.0), 0.0, 255.0) / 255.0;
  return Premultiply(colour);
}

vec4 Tile(ivec2 p)
{
  ivec2 d = p - u_Bounds.xy;
  d -= u_Bounds.zw * ivec2(floor(vec2(d) / vec2(u_Bounds.zw)));
  return Fetch(0u, u_Bounds.xy + d);
}

void main()
{
  ivec2 p = u_Region.xy + ivec2(floor(v_Texel));
  switch (u_Mode.x)
  {
    case KindColourMatrix: o_Colour = ColourMatrix(Fetch(0u, p)); break;
    case KindComponentTransfer: o_Colour = ComponentTransfer(Fetch(0u, p)); break;
    case KindOffset: o_Colour = Offset(p); break;
    case KindFlood: o_Colour = u_P0; break;
    case KindBlend: o_Colour = BlendInputs(p); break;
    case KindComposite: o_Colour = Composite(p); break;
    case KindConvolveMatrix: o_Colour = ConvolveMatrix(p); break;
    case KindMorphology: o_Colour = Morphology(p); break;
    case KindBlur: o_Colour = BlurPass(p); break;
    case KindDisplacementMap: o_Colour = DisplacementMap(p); break;
    case KindDiffuseLighting:
    case KindSpecularLighting: o_Colour = Lighting(p); break;
    case KindTurbulence: o_Colour = Turbulence(p); break;
    case KindTile: o_Colour = Tile(p); break;
    default: o_Colour = vec4(0.0); break;
  }
}
)";

    /// @brief One pass, as the shaders read it (std140).
    struct FilterUniforms
    {
      Array<int32, 4> Region;
      Array<int32, 4> Input0;
      Array<int32, 4> Input1;
      Array<int32, 4> Bounds;
      Array<uint32, 4> Mode;
      Array<uint32, 4> Convert;
      Array<float, 4> TextureSize;
      Array<float, 4> P0;
      Array<float, 4> P1;
      Array<float, 4> P2;
      Array<float, 4> P3;
      Array<Array<float, 4>, 5> Matrix;
      Array<Array<float, 4>, 256> Table;
    };
    static_assert(sizeof(FilterUniforms) == 16u * (11u + 5u + 256u),
                  "FilterUniforms mirrors the shaders' block.");

    enum ReadFlags : uint32
    {
      ReadStraight = 1u << 0u,
      ReadToLinear = 1u << 1u,
      ReadToSrgb = 1u << 2u,
    };

    /// @brief The most kernel values a convolve matrix holds: four to each table entry.
    constexpr size_t MaxKernelValues = 1'024u;
    constexpr uint32 MaxOctaves = 32u;

    // The random number generator and lattice of SVG 1.1's feTurbulence reference code.
    constexpr int32 BlockSize = 256;
    constexpr int32 PerlinNoise = 4'096;
    constexpr int64 RandMaximum = 2'147'483'647;
    constexpr int64 RandAmplitude = 16'807;
    constexpr int64 RandQ = 127'773;
    constexpr int64 RandR = 2'836;

    KRYS_NODISCARD Array<int32, 4> ToArray(const PixelRect2D &rect) noexcept
    {
      return {rect.X, rect.Y, static_cast<int32>(rect.Width), static_cast<int32>(rect.Height)};
    }

    /// @brief How an input in `from` is read by a node in `to`.
    KRYS_NODISCARD uint32 ReadFlagsFor(FilterColourSpace2D from, FilterColourSpace2D to,
                                       bool premultiplied) noexcept
    {
      uint32 flags = premultiplied ? 0u : ReadStraight;
      if (from != to)
      {
        flags |= to == FilterColourSpace2D::LinearSRGB ? ReadToLinear : ReadToSrgb;
      }
      return flags;
    }
  }

  Expected<UniquePtr<FilterRenderer2D>> FilterRenderer2D::Create(Device &device) noexcept
  {
    auto renderer = CreateUnique<FilterRenderer2D>(device);
    if (auto result = renderer->Initialise(); !result)
    {
      return Unexpected<string>(result.error());
    }
    return renderer;
  }

  FilterRenderer2D::FilterRenderer2D(Device &device) noexcept : _device(&device)
  {
  }

  FilterRenderer2D::~FilterRenderer2D() noexcept
  {
    for (const auto &texture : _textures)
    {
      _device->Destroy(texture.Texture);
    }
    for (const auto &table : _turbulenceTables)
    {
      _device->Destroy(table.Texture);
    }
    if (_pipeline.IsValid())
    {
      _device->Destroy(_pipeline);
    }
    for (auto shader : {_vertexShader, _fragmentShader})
    {
      if (shader.IsValid())
      {
        _device->Destroy(shader);
      }
    }
    if (_sampler.IsValid())
    {
      _device->Destroy(_sampler);
    }
    if (_transparentTexture.IsValid())
    {
      _device->Destroy(_transparentTexture);
    }
  }

  Expected<void> FilterRenderer2D::Initialise() noexcept
  {
    auto vertexShader = _device->CreateShader(
      {.Stage = ShaderStage::Vertex, .Source = FilterVertexSource, .DebugName = u8"Render2D.Filter.vert"});
    if (!vertexShader)
    {
      return Unexpected<string>(vertexShader.error());
    }
    _vertexShader = *vertexShader;

    auto fragmentShader = _device->CreateShader({.Stage = ShaderStage::Fragment,
                                                 .Source = FilterFragmentSource,
                                                 .DebugName = u8"Render2D.Filter.frag"});
    if (!fragmentShader)
    {
      return Unexpected<string>(fragmentShader.error());
    }
    _fragmentShader = *fragmentShader;

    // Each pass replaces what's in its texture: no blending, and the quad covers the region.
    auto pipeline = _device->CreatePipeline({
      .VertexShader = _vertexShader,
      .FragmentShader = _fragmentShader,
      .Topology = PrimitiveTopology::TriangleStrip,
      .Blend = {},
      .Raster = {.ScissorTest = false},
      .ColourFormats = {Format},
      .DebugName = u8"Render2D.Filter",
    });
    if (!pipeline)
    {
      return Unexpected<string>(pipeline.error());
    }
    _pipeline = *pipeline;

    auto sampler = _device->CreateSampler(SamplerDesc::NearestClamp());
    if (!sampler)
    {
      return Unexpected<string>(sampler.error());
    }
    _sampler = *sampler;

    auto transparent =
      _device->CreateTexture({.Width = 1u, .Height = 1u, .DebugName = u8"Render2D.Filter.Transparent"});
    if (!transparent)
    {
      return Unexpected<string>(transparent.error());
    }
    _transparentTexture = *transparent;
    constexpr Array<byte, 4> clear {};
    _device->WriteTexture(_transparentTexture, TextureRegion::Whole2D(1u, 1u), clear);
    return {};
  }

  Maybe<FilterResult2D> FilterRenderer2D::Render(CommandBuffer &commands, const FilterGraph2D &graph) noexcept
  {
    if (graph.Nodes.empty())
    {
      return null;
    }

    // Textures unused for a while go.
    const uint64 frame = _device->FrameNumber();
    std::erase_if(_textures,
                  [&](const PooledTexture &texture)
                  {
                    if (texture.LastUsedFrame + 120u < frame)
                    {
                      _device->Destroy(texture.Texture);
                      return true;
                    }
                    return false;
                  });

    struct NodeResult
    {
      TextureHandle Texture {};
      PixelRect2D Region {};
      FilterColourSpace2D ColourSpace {FilterColourSpace2D::SRGB};
    };
    List<NodeResult> results(graph.Nodes.size());

    // What an input reads: its texture, region, and how it's read in the node's colour space.
    struct Read
    {
      TextureHandle Texture {};
      PixelRect2D Region {};
      uint32 Flags {0u};
    };
    auto readOf = [&](const FilterInput2D &input, FilterColourSpace2D space) -> Read
    {
      switch (input.Type)
      {
        case FilterInput2D::Kind::Source:
        {
          if (input.Index >= graph.Sources.size())
          {
            break;
          }
          const auto &source = graph.Sources[input.Index];
          return {.Texture = source.Texture,
                  .Region = source.Region,
                  .Flags = ReadFlagsFor(source.ColourSpace, space, source.Premultiplied)};
        }
        case FilterInput2D::Kind::Node:
        {
          if (input.Index >= results.size() || !results[input.Index].Texture.IsValid())
          {
            break;
          }
          const auto &result = results[input.Index];
          return {.Texture = result.Texture,
                  .Region = result.Region,
                  .Flags = ReadFlagsFor(result.ColourSpace, space, true)};
        }
        case FilterInput2D::Kind::None: break;
      }
      return {.Texture = _transparentTexture, .Region = {}, .Flags = 0u};
    };

    auto recordPass = [&](TextureHandle destination, const FilterUniforms &uniforms, TextureHandle source0,
                          TextureHandle source1, TextureHandle turbulence)
    {
      auto block = _device->AllocateTransient(sizeof(FilterUniforms));
      if (!block.IsValid())
      {
        return;
      }
      std::memcpy(block.Data.data(), &uniforms, sizeof(uniforms));
      const auto desc = _device->GetTextureDesc(destination);
      commands.BeginRenderPass(RenderPassDesc::SingleColour(
        {.Texture = destination, .Load = AttachmentLoadOp::Clear, .ClearColour = {}}));
      commands.SetViewport(0.f, 0.f, static_cast<float>(desc->Width), static_cast<float>(desc->Height));
      commands.BindPipeline(_pipeline);
      commands.BindUniformBuffer(0u, block.Buffer, block.Offset, sizeof(FilterUniforms));
      commands.BindTexture(0u, source0.IsValid() ? source0 : _transparentTexture, _sampler);
      commands.BindTexture(1u, source1.IsValid() ? source1 : _transparentTexture, _sampler);
      commands.BindTexture(2u, turbulence.IsValid() ? turbulence : _transparentTexture, _sampler);
      commands.Draw(4u);
      commands.EndRenderPass();
    };

    for (size_t index = 0u; index < graph.Nodes.size(); ++index)
    {
      const auto &node = graph.Nodes[index];
      auto &result = results[index];
      result.Region = node.Region;
      result.ColourSpace = node.ColourSpace;
      if (node.Region.IsEmpty())
      {
        continue;
      }
      result.Texture = AcquireTexture(node.Region.Width, node.Region.Height);
      if (!result.Texture.IsValid())
      {
        continue;
      }
      const auto desc = _device->GetTextureDesc(result.Texture);

      const auto input0 = readOf(node.Inputs[0], node.ColourSpace);
      auto input1 = readOf(node.Inputs[1], node.ColourSpace);

      FilterUniforms uniforms {};
      uniforms.Region = ToArray(node.Region);
      uniforms.Input0 = ToArray(input0.Region);
      uniforms.Input1 = ToArray(input1.Region);
      uniforms.Bounds = ToArray(node.Region);
      uniforms.Mode = {static_cast<uint32>(node.Type), 0u, 0u, 0u};
      uniforms.Convert = {input0.Flags, input1.Flags, static_cast<uint32>(node.EdgeMode), 0u};
      uniforms.TextureSize = {static_cast<float>(desc->Width), static_cast<float>(desc->Height), 0.f, 0.f};
      TextureHandle turbulence {};

      switch (node.Type)
      {
        case FilterNode2D::Kind::ColourMatrix:
        {
          for (uint32 row = 0u; row < 4u; ++row)
          {
            for (uint32 column = 0u; column < 4u; ++column)
            {
              uniforms.Matrix[row][column] = node.Matrix[row * 5u + column];
            }
            uniforms.Matrix[4][row] = node.Matrix[row * 5u + 4u];
          }
          break;
        }
        case FilterNode2D::Kind::ComponentTransfer:
        {
          for (size_t i = 0u; i < 256u; ++i)
          {
            for (size_t channel = 0u; channel < 4u; ++channel)
            {
              const float value =
                i < node.Table.size() ? static_cast<float>(node.Table[i][channel]) : static_cast<float>(i);
              uniforms.Table[i][channel] = value / 255.f;
            }
          }
          break;
        }
        case FilterNode2D::Kind::Offset: uniforms.P0 = {node.Offset[0], node.Offset[1], 0.f, 0.f}; break;
        case FilterNode2D::Kind::Flood:  uniforms.P0 = node.Colour; break;
        case FilterNode2D::Kind::Blend:  uniforms.Mode[1] = static_cast<uint32>(node.Blend); break;
        case FilterNode2D::Kind::Composite:
          uniforms.Mode[1] = static_cast<uint32>(node.Operator);
          uniforms.P0 = node.K;
          break;
        case FilterNode2D::Kind::ConvolveMatrix:
        {
          const size_t count = static_cast<size_t>(node.Order[0]) * node.Order[1];
          if (count == 0u || count > MaxKernelValues || node.Kernel.size() < count || node.Divisor == 0.f)
          {
            continue;
          }
          uniforms.Mode[1] = node.Order[0];
          uniforms.Mode[2] = node.Order[1];
          uniforms.P0 = {node.Divisor, node.Bias, node.PreserveAlpha ? 1.f : 0.f, 0.f};
          uniforms.P1 = {static_cast<float>(node.Target[0]), static_cast<float>(node.Target[1]), 0.f, 0.f};
          for (size_t i = 0u; i < count; ++i)
          {
            uniforms.Table[i / 4u][i % 4u] = node.Kernel[i];
          }
          break;
        }
        case FilterNode2D::Kind::Morphology:
        case FilterNode2D::Kind::Blur:
        {
          // Two passes, across into a scratch texture over the region, then down from it.
          const auto scratch = AcquireTexture(node.Region.Width, node.Region.Height);
          if (!scratch.IsValid())
          {
            continue;
          }
          if (node.Type == FilterNode2D::Kind::Morphology)
          {
            uniforms.Mode[1] = node.Dilate ? 1u : 0u;
            uniforms.P0 = {static_cast<float>(node.Radius[0]), static_cast<float>(node.Radius[1]), 0.f, 0.f};
          }
          else
          {
            uniforms.P0 = {node.StandardDeviation[0], node.StandardDeviation[1], 0.f, 0.f};
          }
          const auto scratchDesc = _device->GetTextureDesc(scratch);
          auto across = uniforms;
          across.TextureSize = {static_cast<float>(scratchDesc->Width),
                                static_cast<float>(scratchDesc->Height), 0.f, 0.f};
          recordPass(scratch, across, input0.Texture, {}, {});
          auto down = uniforms;
          down.Mode[3] = 1u;
          down.Input0 = ToArray(node.Region);
          down.Convert[0] = 0u;
          recordPass(result.Texture, down, scratch, {}, {});
          continue;
        }
        case FilterNode2D::Kind::DisplacementMap:
          uniforms.Mode[1] = static_cast<uint32>(node.XChannel);
          uniforms.Mode[2] = static_cast<uint32>(node.YChannel);
          uniforms.P0 = {node.Offset[0], node.Offset[1], 0.f, 0.f};
          break;
        case FilterNode2D::Kind::DiffuseLighting:
        case FilterNode2D::Kind::SpecularLighting:
          uniforms.Mode[1] = static_cast<uint32>(node.Light.Type);
          uniforms.P0 = {node.SurfaceScale, node.DiffuseConstant, node.SpecularConstant,
                         node.SpecularExponent};
          uniforms.P1 = {node.LightingColour[0], node.LightingColour[1], node.LightingColour[2], 0.f};
          uniforms.P2 = {node.Light.Vector[0], node.Light.Vector[1], node.Light.Vector[2], 0.f};
          uniforms.P3 = {node.Light.Direction[0], node.Light.Direction[1], node.Light.Direction[2],
                         node.Light.SpecularExponent};
          uniforms.Matrix[0] = {node.Light.ConeCutOff, node.Light.ConeFullLight, 0.f, 0.f};
          break;
        case FilterNode2D::Kind::Turbulence:
        {
          // Stitching adjusts the frequencies so the tile's edges join.
          auto frequency = node.BaseFrequency;
          Array<int32, 4> stitch {};
          if (node.StitchTiles)
          {
            const float tileWidth = std::round(node.StitchTileSize[0]);
            const float tileHeight = std::round(node.StitchTileSize[1]);
            auto adjust = [](float &value, float tile)
            {
              if (value == 0.f || tile <= 0.f)
              {
                return;
              }
              const float low = std::floor(tile * value) / tile;
              const float high = std::ceil(tile * value) / tile;
              value = value / low < high / value ? low : high;
            };
            adjust(frequency[0], tileWidth);
            adjust(frequency[1], tileHeight);
            stitch[0] = static_cast<int32>(std::round(tileWidth * frequency[0]));
            stitch[1] = static_cast<int32>(std::round(tileHeight * frequency[1]));
            stitch[2] = PerlinNoise + stitch[0];
            stitch[3] = PerlinNoise + stitch[1];
          }
          uniforms.Mode[1] = node.FractalNoise ? 1u : 0u;
          uniforms.Mode[2] = std::min(node.Octaves, MaxOctaves);
          uniforms.P0 = {frequency[0], frequency[1], node.Scale[0], node.Scale[1]};
          uniforms.P1 = {static_cast<float>(stitch[0]), static_cast<float>(stitch[1]),
                         static_cast<float>(stitch[2]), static_cast<float>(stitch[3])};
          uniforms.P2 = {node.StitchTiles ? 1.f : 0.f, 0.f, 0.f, 0.f};
          turbulence = TurbulenceTableFor(node.Seed);
          break;
        }
        case FilterNode2D::Kind::Tile:
          if (node.TileRect.IsEmpty())
          {
            continue;
          }
          uniforms.Bounds = ToArray(node.TileRect);
          break;
      }
      recordPass(result.Texture, uniforms, input0.Texture, input1.Texture, turbulence);
    }

    const auto &last = results.back();
    if (!last.Texture.IsValid())
    {
      return null;
    }
    return FilterResult2D {.Texture = last.Texture, .Region = last.Region, .ColourSpace = last.ColourSpace};
  }

  TextureHandle FilterRenderer2D::AcquireTexture(uint32 width, uint32 height) noexcept
  {
    // Any texture this frame hasn't used that's large enough, the smallest first: results are drawn by the
    // frame's passes, recorded later, so none can be reused within it.
    const uint64 frame = _device->FrameNumber();
    PooledTexture *best = nullptr;
    for (auto &texture : _textures)
    {
      if (texture.LastUsedFrame < frame && texture.Width >= width && texture.Height >= height
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

    // New textures round up to 64 pixels, so slightly different nodes can reuse them.
    const uint32 roundedWidth = (width + 63u) / 64u * 64u;
    const uint32 roundedHeight = (height + 63u) / 64u * 64u;
    auto texture = _device->CreateTexture({.Format = Format,
                                           .Width = roundedWidth,
                                           .Height = roundedHeight,
                                           .Usage = TextureUsage::RenderTarget | TextureUsage::Sampled,
                                           .DebugName = u8"Render2D.Filter"});
    if (!texture)
    {
      KRYS_LOG_ERROR("Render2D: {}", texture.error());
      return {};
    }
    _textures.push_back(
      {.Texture = *texture, .Width = roundedWidth, .Height = roundedHeight, .LastUsedFrame = frame});
    return *texture;
  }

  TextureHandle FilterRenderer2D::TurbulenceTableFor(int32 requestedSeed) noexcept
  {
    // The seed is clamped to [1, RandMaximum - 1].
    int64 seed = requestedSeed;
    if (seed <= 0)
    {
      seed = -(seed % (RandMaximum - 1)) + 1;
    }
    if (seed > RandMaximum - 1)
    {
      seed = RandMaximum - 1;
    }

    const uint64 frame = _device->FrameNumber();
    for (auto &table : _turbulenceTables)
    {
      if (table.Seed == static_cast<int32>(seed))
      {
        table.LastUsedFrame = frame;
        return table.Texture;
      }
    }

    const int32 clampedSeed = static_cast<int32>(seed);
    auto random = [&]
    {
      int64 result = RandAmplitude * (seed % RandQ) - RandR * (seed / RandQ);
      if (result <= 0)
      {
        result += RandMaximum;
      }
      seed = result;
      return result;
    };

    constexpr int32 entries = 2 * BlockSize + 2;
    Array<int32, entries> lattice {};
    Array<Array<Array<float, 2>, entries>, 4> gradient {};
    for (int32 channel = 0; channel < 4; ++channel)
    {
      for (int32 i = 0; i < BlockSize; ++i)
      {
        lattice[i] = i;
        auto &g = gradient[channel][i];
        do
        {
          g[0] = static_cast<float>((random() % (2 * BlockSize)) - BlockSize) / BlockSize;
          g[1] = static_cast<float>((random() % (2 * BlockSize)) - BlockSize) / BlockSize;
        } while (g[0] == 0.f && g[1] == 0.f);
        const float length = std::hypot(g[0], g[1]);
        g[0] /= length;
        g[1] /= length;
      }
    }
    for (int32 i = BlockSize - 1; i > 0; --i)
    {
      const int32 k = lattice[i];
      const auto j = static_cast<int32>(random() % BlockSize);
      lattice[i] = lattice[j];
      lattice[j] = k;
    }
    for (int32 i = 0; i < BlockSize + 2; ++i)
    {
      lattice[BlockSize + i] = lattice[i];
      for (int32 channel = 0; channel < 4; ++channel)
      {
        gradient[channel][BlockSize + i] = gradient[channel][i];
      }
    }

    // Row 0 holds the lattice; rows 1 and 2 the gradients of channels 0 and 1, then 2 and 3.
    List<float> texels(static_cast<size_t>(entries) * 3u * 4u, 0.f);
    for (int32 i = 0; i < entries; ++i)
    {
      texels[static_cast<size_t>(i) * 4u] = static_cast<float>(lattice[i]);
      for (int32 channel = 0; channel < 4; ++channel)
      {
        const size_t row = 1u + static_cast<size_t>(channel) / 2u;
        const size_t at = (row * entries + static_cast<size_t>(i)) * 4u + (channel % 2 == 0 ? 0u : 2u);
        texels[at] = gradient[channel][i][0];
        texels[at + 1u] = gradient[channel][i][1];
      }
    }
    auto texture = _device->CreateTexture({.Format = TextureFormat::RGBA32Float,
                                           .Width = static_cast<uint32>(entries),
                                           .Height = 3u,
                                           .DebugName = u8"Render2D.Filter.Turbulence"});
    if (!texture)
    {
      KRYS_LOG_ERROR("Render2D: {}", texture.error());
      return {};
    }
    _device->WriteTexture(
      *texture, TextureRegion::Whole2D(static_cast<uint32>(entries), 3u),
      Span<const byte>(reinterpret_cast<const byte *>(texels.data()), texels.size() * sizeof(float)));

    // A few seeds are kept; the least recently used goes.
    constexpr size_t maxTables = 8u;
    if (_turbulenceTables.size() >= maxTables)
    {
      auto oldest = std::ranges::min_element(_turbulenceTables, {}, &TurbulenceTable::LastUsedFrame);
      _device->Destroy(oldest->Texture);
      _turbulenceTables.erase(oldest);
    }
    _turbulenceTables.push_back({.Seed = clampedSeed, .Texture = *texture, .LastUsedFrame = frame});
    return *texture;
  }
}
