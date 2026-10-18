#include "Krystal.Gfx/Render2D/Renderer2D.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Log/ILogger.hpp"
#include "Krystal.Core/Utils/Move.hpp"
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
  uint u_DecodeSrgb;
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
layout(location = 5) flat out vec4 v_Colour;           // premultiplied, in the target's blending space
layout(location = 6) out vec2 v_UV;
layout(location = 7) flat out vec4 v_UVRect;
layout(location = 8) flat out uint v_Flags;
layout(location = 9) flat out vec4 v_Hole; // the hole's centre relative to the rect's, and its half size
layout(location = 10) flat out vec4 v_HoleRadii;
layout(location = 11) out vec2 v_Pixel; // in target pixels

vec3 SrgbToLinear(vec3 c)
{
  return mix(c / 12.92, pow((c + 0.055) / 1.055, vec3(2.4)), greaterThan(c, vec3(0.04045)));
}

void main()
{
  mat2 linear = mat2(a_Linear.xy, a_Linear.zw);
  mat2 inverseLinear = inverse(linear);
  // How far one target pixel reaches along each local axis.
  vec2 pixelInLocal = vec2(length(vec2(inverseLinear[0][0], inverseLinear[1][0])),
                           length(vec2(inverseLinear[0][1], inverseLinear[1][1])));

  vec2 corner = vec2(gl_VertexID & 1, gl_VertexID >> 1);
  vec2 halfSize = a_Rect.zw * 0.5;
  vec2 position = (corner * 2.0 - 1.0) * (halfSize + pixelInLocal);
  vec2 local = a_Rect.xy + halfSize + position;
  vec2 pixel = linear * local + a_Translation;
  vec2 ndc = pixel / u_ViewportSize * 2.0 - 1.0;
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

  // sRGB targets blend in linear light and encode on write; other targets blend the encoded values.
  vec3 rgb = u_DecodeSrgb != 0u ? SrgbToLinear(a_Colour.rgb) : a_Colour.rgb;
  v_Colour = vec4(rgb * a_Colour.a, a_Colour.a);
}
)";

    constexpr const char8_t *RectFragmentSource = u8R"(#version 450
layout(binding = 0, std140) uniform Frame
{
  vec2 u_ViewportSize;
  uint u_DecodeSrgb;
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

layout(location = 0) out vec4 o_Colour;

const uint Textured = 1u;
const uint StraightAlpha = 2u;
const uint LinearTexels = 4u;
const uint Mask = 8u;
const uint Hole = 16u;

vec3 SrgbToLinear(vec3 c)
{
  return mix(c / 12.92, pow((c + 0.055) / 1.055, vec3(2.4)), greaterThan(c, vec3(0.04045)));
}

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

// A texel, premultiplied and in the target's blending space.
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

  if ((v_Flags & StraightAlpha) != 0u)
  {
    texel.rgb *= texel.a;
  }

  bool linearTexels = (v_Flags & LinearTexels) != 0u;
  bool linearTarget = u_DecodeSrgb != 0u;
  if (linearTexels != linearTarget && texel.a > 0.0)
  {
    vec3 rgb = texel.rgb / texel.a;
    texel.rgb = (linearTarget ? SrgbToLinear(rgb) : LinearToSrgb(rgb)) * texel.a;
  }
  return texel;
}

void main()
{
  // Derivatives are taken here, outside the branch, where every fragment of the quad runs them.
  vec2 dx = dFdx(v_UV);
  vec2 dy = dFdy(v_UV);

  float coverage = Coverage(v_Position, v_HalfSize, v_Radii, v_InverseTranspose);
  if ((v_Flags & Hole) != 0u)
  {
    coverage *= 1.0 - Coverage(v_Position - v_Hole.xy, v_Hole.zw, v_HoleRadii, v_InverseTranspose);
  }
  coverage *= ClipCoverage();

  vec4 colour = v_Colour;
  if ((v_Flags & Textured) != 0u)
  {
    colour *= SampleTexture(dx, dy);
  }
  o_Colour = colour * coverage;
}
)";

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
      : _device(&device), _masks(device, desc.Masks)
  {
  }

  Renderer2D::~Renderer2D() noexcept
  {
    for (const auto &entry : _pipelines)
    {
      _device->Destroy(entry.Pipeline);
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
    _commands = &commands;
    _target = target;
    _state = {.Transform = {},
              .Clip = {.X = 0, .Y = 0, .Width = target.Width, .Height = target.Height},
              .Chain = NoChain};
    _chains.clear();
    _savedStates.clear();
    _rects.clear();
    _batches.clear();
    _masks.ResetStats();
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

  void Renderer2D::FillRect(const Rect2D &rect, SRGBA<uint8> colour) noexcept
  {
    FillRoundedRect({.Rect = rect, .Radii = {}}, colour);
  }

  void Renderer2D::FillRoundedRect(const RoundedRect2D &roundedRect, SRGBA<uint8> colour) noexcept
  {
    krys_debug_assert_msg(_commands != nullptr, "FillRoundedRect outside Begin and End.");
    const auto &rect = roundedRect.Rect;
    if (rect.Width <= 0.f || rect.Height <= 0.f)
    {
      return;
    }

    const auto resolved = colour.Resolved();
    if (resolved.Alpha == 0u)
    {
      return;
    }

    AddRoundedRect(roundedRect, {resolved.Red, resolved.Green, resolved.Blue, resolved.Alpha},
                   _state.Transform);
  }

  void Renderer2D::AddRoundedRect(const RoundedRect2D &roundedRect, Array<uint8, 4> colour,
                                  const Transform2D &transform) noexcept
  {
    const auto &rect = roundedRect.Rect;
    if (rect.Width <= 0.f || rect.Height <= 0.f)
    {
      return;
    }
    const auto radii = ConstrainRadii(rect, roundedRect.Radii);
    AddInstance({.Rect = {rect.X, rect.Y, rect.Width, rect.Height},
                 .Radii = {radii.TopLeft, radii.TopRight, radii.BottomRight, radii.BottomLeft},
                 .Linear = {},
                 .Translation = {},
                 .UV = {},
                 .HoleRect = {},
                 .HoleRadii = {},
                 .Colour = colour,
                 .Flags = 0u},
                transform, {}, {});
  }

  void Renderer2D::StrokePath(const Path2D &path, const StrokeStyle &style, SRGBA<uint8> colour) noexcept
  {
    krys_debug_assert_msg(_commands != nullptr, "StrokePath outside Begin and End.");
    if (colour.Resolved().Alpha == 0u || path.IsEmpty() || _state.Clip.IsEmpty())
    {
      return;
    }
    const auto outline = _stroker.Stroke(path, style, _state.Transform);
    if (!outline.IsEmpty())
    {
      FillPath(outline, FillRule::NonZero, colour);
    }
  }

  void Renderer2D::StrokeLine(Vec2f from, Vec2f to, const StrokeStyle &style, SRGBA<uint8> colour) noexcept
  {
    krys_debug_assert_msg(_commands != nullptr, "StrokeLine outside Begin and End.");
    const auto resolved = colour.Resolved();
    if (resolved.Alpha == 0u || _state.Clip.IsEmpty())
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
    const Array<uint8, 4> packed {resolved.Red, resolved.Green, resolved.Blue, resolved.Alpha};
    auto addDash = [&](float start, float end)
    {
      AddRoundedRect({.Rect = {start - extension, -halfWidth, end - start + 2.f * extension, 2.f * halfWidth},
                      .Radii = CornerRadii2D::Uniform(radius)},
                     packed, transform);
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
                                           SRGBA<uint8> colour) noexcept
  {
    krys_debug_assert_msg(_commands != nullptr, "FillRectWithRoundedHole outside Begin and End.");
    const auto &holeRect = hole.Rect;
    if (holeRect.Width <= 0.f || holeRect.Height <= 0.f)
    {
      FillRect(rect, colour);
      return;
    }
    if (rect.Width <= 0.f || rect.Height <= 0.f)
    {
      return;
    }

    const auto resolved = colour.Resolved();
    if (resolved.Alpha == 0u)
    {
      return;
    }

    const auto radii = ConstrainRadii(holeRect, hole.Radii);
    AddInstance({.Rect = {rect.X, rect.Y, rect.Width, rect.Height},
                 .Radii = {},
                 .Linear = {},
                 .Translation = {},
                 .UV = {},
                 .HoleRect = {holeRect.X, holeRect.Y, holeRect.Width, holeRect.Height},
                 .HoleRadii = {radii.TopLeft, radii.TopRight, radii.BottomRight, radii.BottomLeft},
                 .Colour = {resolved.Red, resolved.Green, resolved.Blue, resolved.Alpha},
                 .Flags = Hole},
                _state.Transform, {}, {});
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

  void Renderer2D::FillPath(const Path2D &path, FillRule rule, SRGBA<uint8> colour) noexcept
  {
    krys_debug_assert_msg(_commands != nullptr, "FillPath outside Begin and End.");
    const auto resolved = colour.Resolved();
    if (resolved.Alpha == 0u || path.IsEmpty() || _state.Clip.IsEmpty())
    {
      return;
    }

    const auto mask = _masks.GetMask(path, _state.Transform, rule, _state.Clip);
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
                 .Colour = {resolved.Red, resolved.Green, resolved.Blue, resolved.Alpha},
                 .Flags = Textured | Mask},
                Transform2D::Identity(), mask.Texture, _nearestSampler);
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
                               SamplerHandle sampler) noexcept
  {
    // Nothing shows through an empty clip or a transform that flattens everything.
    if (_state.Clip.IsEmpty() || transform.Determinant() == 0.f)
    {
      return;
    }
    instance.Linear = {transform.A, transform.B, transform.C, transform.D};
    instance.Translation = {transform.E, transform.F};

    // Untextured instances join any batch with the same scissor; a textured one needs the batch's texture,
    // or a batch that has none yet.
    const bool textured = texture.IsValid();
    auto joins = [&](const Batch &batch)
    {
      return batch.Scissor == _state.Clip && batch.Chain == _state.Chain
             && (!textured || !batch.Texture.IsValid()
                 || (batch.Texture == texture && batch.Sampler == sampler));
    };
    if (_batches.empty() || !joins(_batches.back()))
    {
      _batches.push_back({.FirstInstance = static_cast<uint32>(_rects.size()),
                          .InstanceCount = 0u,
                          .Scissor = _state.Clip,
                          .Chain = _state.Chain,
                          .Texture = {},
                          .Sampler = {}});
    }

    auto &batch = _batches.back();
    if (textured && !batch.Texture.IsValid())
    {
      batch.Texture = texture;
      batch.Sampler = sampler;
    }
    ++batch.InstanceCount;
    _rects.push_back(instance);
  }

  void Renderer2D::End() noexcept
  {
    krys_assert_msg(_commands != nullptr, "End without Begin.");
    krys_debug_assert_msg(_savedStates.empty(), "Save without a matching Restore.");
    auto &commands = *_commands;
    _commands = nullptr;

    ColourAttachment attachment {.Texture = _target.Texture, .Swapchain = _target.Swapchain};
    attachment.Load = _target.ClearColour ? AttachmentLoadOp::Clear : AttachmentLoadOp::Load;
    if (_target.ClearColour)
    {
      attachment.ClearColour = *_target.ClearColour;
    }
    commands.BeginRenderPass(RenderPassDesc::SingleColour(attachment));

    if (!_rects.empty() && _target.Width > 0u && _target.Height > 0u)
    {
      auto pipeline = GetPipeline(_target.Format);
      const uint64 rectBytes = _rects.size() * sizeof(RectInstance);
      auto frame = _device->AllocateTransient(sizeof(FrameUniforms));
      auto rects = _device->AllocateTransient(rectBytes, alignof(RectInstance));
      if (!pipeline)
      {
        KRYS_LOG_ERROR("Render2D: {}", pipeline.error());
      }
      else if (frame.IsValid() && rects.IsValid())
      {
        const FrameUniforms uniforms {
          .ViewportSize = {static_cast<float>(_target.Width), static_cast<float>(_target.Height)},
          .DecodeSrgb = IsSrgbFormat(_target.Format) ? 1u : 0u,
          .Padding = 0u};
        std::memcpy(frame.Data.data(), &uniforms, sizeof(uniforms));
        std::memcpy(rects.Data.data(), _rects.data(), rectBytes);

        commands.BindPipeline(*pipeline);
        commands.BindUniformBuffer(0u, frame.Buffer, frame.Offset, sizeof(FrameUniforms));
        commands.BindVertexBuffer(0u, rects.Buffer, rects.Offset);
        TextureHandle boundTexture {};
        SamplerHandle boundSampler {};
        uint64 boundChain = ~0ull;
        for (const auto &batch : _batches)
        {
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
                data.Clips[i] = {
                  .Rect = {rect.X, rect.Y, rect.Width, rect.Height},
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

          // Untextured batches never sample, but drivers still check the sampler's texture at every draw,
          // so they get the white texture unless another one is already bound.
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
          commands.SetScissor(batch.Scissor.X, batch.Scissor.Y, batch.Scissor.Width, batch.Scissor.Height);
          commands.Draw(4u, batch.InstanceCount, 0u, batch.FirstInstance);
        }
      }
    }

    commands.EndRenderPass();
    _stats = {.Instances = static_cast<uint32>(_rects.size()),
              .Batches = static_cast<uint32>(_batches.size()),
              .MasksRasterised = _masks.Stats().Rasterised,
              .MaskCacheHits = _masks.Stats().Hits};
    _savedStates.clear();
    _rects.clear();
    _batches.clear();
    _chains.clear();
  }

  Expected<PipelineHandle> Renderer2D::GetPipeline(TextureFormat format) noexcept
  {
    for (const auto &entry : _pipelines)
    {
      if (entry.Format == format)
      {
        return entry.Pipeline;
      }
    }

    PipelineDesc desc {
      .VertexShader = _vertexShader,
      .FragmentShader = _fragmentShader,
      .Topology = PrimitiveTopology::TriangleStrip,
      .Blend = BlendState::PremultipliedAlpha(),
      .Raster = {.ScissorTest = true},
      .ColourFormats = {format},
      .DebugName = u8"Render2D.Rect",
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
    _pipelines.push_back({.Format = format, .Pipeline = *pipeline});
    return *pipeline;
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
