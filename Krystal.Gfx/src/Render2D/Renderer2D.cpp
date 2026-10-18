#include "Krystal.Gfx/Render2D/Renderer2D.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Log/ILogger.hpp"
#include "Krystal.Core/Utils/Move.hpp"
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
layout(location = 4) in vec4 a_Colour;      // sRGB, straight alpha

layout(location = 0) out vec2 v_Position; // relative to the rect's centre, in local coordinates
layout(location = 1) flat out vec2 v_HalfSize;
layout(location = 2) flat out vec4 v_Radii;
layout(location = 3) flat out mat2 v_InverseTranspose; // takes locations 3 and 4
layout(location = 5) flat out vec4 v_Colour;

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
  vec2 pixel = linear * (a_Rect.xy + halfSize + position) + a_Translation;
  vec2 ndc = pixel / u_ViewportSize * 2.0 - 1.0;
  gl_Position = vec4(ndc.x, -ndc.y, 0.0, 1.0);

  v_Position = position;
  v_HalfSize = halfSize;
  v_Radii = a_Radii;
  v_InverseTranspose = transpose(inverseLinear);

  // sRGB targets blend in linear light and encode on write; other targets blend the encoded values.
  vec3 rgb = u_DecodeSrgb != 0u ? SrgbToLinear(a_Colour.rgb) : a_Colour.rgb;
  v_Colour = vec4(rgb * a_Colour.a, a_Colour.a);
}
)";

    constexpr const char8_t *RectFragmentSource = u8R"(#version 450
layout(location = 0) in vec2 v_Position;
layout(location = 1) flat in vec2 v_HalfSize;
layout(location = 2) flat in vec4 v_Radii;
layout(location = 3) flat in mat2 v_InverseTranspose;
layout(location = 5) flat in vec4 v_Colour;

layout(location = 0) out vec4 o_Colour;

// The signed distance from p to the edge of a rounded box centred on the origin (negative inside), and the
// direction in which it grows fastest. y points down, so the top corners are where p.y < 0.
float RoundedBoxDistance(vec2 p, vec2 halfSize, vec4 radii, out vec2 normal)
{
  float r = p.x < 0.0 ? (p.y < 0.0 ? radii.x : radii.w) : (p.y < 0.0 ? radii.y : radii.z);
  vec2 q = abs(p) - halfSize + r;
  vec2 folded = q.x > 0.0 && q.y > 0.0 ? normalize(q) : (q.x > q.y ? vec2(1.0, 0.0) : vec2(0.0, 1.0));
  normal = folded * vec2(p.x < 0.0 ? -1.0 : 1.0, p.y < 0.0 ? -1.0 : 1.0);
  return min(max(q.x, q.y), 0.0) + length(max(q, 0.0)) - r;
}

void main()
{
  vec2 normal;
  float distance = RoundedBoxDistance(v_Position, v_HalfSize, v_Radii, normal);
  // The distance grows by this much in local units per target pixel, so this converts it to pixels. It's
  // exactly 1 without a transform, which keeps pixel-aligned edges sharp.
  float localPerPixel = length(v_InverseTranspose * normal);
  // A one-pixel antialiasing band centred on the edge.
  float coverage = clamp(0.5 - distance / localPerPixel, 0.0, 1.0);
  o_Colour = v_Colour * coverage;
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

  Expected<UniquePtr<Renderer2D>> Renderer2D::Create(Device &device) noexcept
  {
    auto renderer = CreateUnique<Renderer2D>(device);
    if (auto result = renderer->Initialise(); !result)
    {
      return Unexpected<string>(result.error());
    }
    return renderer;
  }

  Renderer2D::Renderer2D(Device &device) noexcept : _device(&device)
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
    return {};
  }

  void Renderer2D::Begin(CommandBuffer &commands, const Render2DTarget &target) noexcept
  {
    krys_assert_msg(_commands == nullptr, "Begin called twice without End.");
    krys_assert_msg(target.Swapchain.IsValid() || target.Texture.IsValid(),
                    "The target needs a texture or swapchain.");
    _commands = &commands;
    _target = target;
    _state = {.Transform = {}, .Clip = {.X = 0, .Y = 0, .Width = target.Width, .Height = target.Height}};
    _savedStates.clear();
    _rects.clear();
    _batches.clear();
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
    const auto &transform = _state.Transform;
    // FIXME: clip to the rotated or skewed rect itself (an antialiased clip mask, G2g), not its bounding box.
    krys_debug_assert_msg(transform.PreservesAxisAlignment(),
                          "Clips under rotations and skews aren't supported yet.");

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
    _state.Clip = Intersect(_state.Clip, ToPixelRect(left, top, right, bottom));
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

    // Nothing shows through an empty clip or a transform that flattens everything.
    const auto &transform = _state.Transform;
    if (_state.Clip.Width == 0u || _state.Clip.Height == 0u || transform.Determinant() == 0.f)
    {
      return;
    }

    if (_batches.empty() || _batches.back().Scissor != _state.Clip)
    {
      _batches.push_back(
        {.FirstInstance = static_cast<uint32>(_rects.size()), .InstanceCount = 0u, .Scissor = _state.Clip});
    }
    ++_batches.back().InstanceCount;

    const auto radii = ConstrainRadii(rect, roundedRect.Radii);
    _rects.push_back({.Rect = {rect.X, rect.Y, rect.Width, rect.Height},
                      .Radii = {radii.TopLeft, radii.TopRight, radii.BottomRight, radii.BottomLeft},
                      .Linear = {transform.A, transform.B, transform.C, transform.D},
                      .Translation = {transform.E, transform.F},
                      .Colour = {resolved.Red, resolved.Green, resolved.Blue, resolved.Alpha}});
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
        for (const auto &batch : _batches)
        {
          commands.SetScissor(batch.Scissor.X, batch.Scissor.Y, batch.Scissor.Width, batch.Scissor.Height);
          commands.Draw(4u, batch.InstanceCount, 0u, batch.FirstInstance);
        }
      }
    }

    commands.EndRenderPass();
    _savedStates.clear();
    _rects.clear();
    _batches.clear();
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
                     {4u, VertexFormat::UByte4Norm, offsetof(RectInstance, Colour)}},
    });

    auto pipeline = _device->CreatePipeline(desc);
    if (!pipeline)
    {
      return Unexpected<string>(pipeline.error());
    }
    _pipelines.push_back({.Format = format, .Pipeline = *pipeline});
    return *pipeline;
  }

  Renderer2D::PixelRect Renderer2D::ToPixelRect(float left, float top, float right, float bottom) noexcept
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

  Renderer2D::PixelRect Renderer2D::Intersect(const PixelRect &a, const PixelRect &b) noexcept
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
