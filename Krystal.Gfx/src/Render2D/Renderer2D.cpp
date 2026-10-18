#include "Krystal.Gfx/Render2D/Renderer2D.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Log/ILogger.hpp"
#include "Krystal.Core/Utils/Move.hpp"
#include <cstddef>
#include <cstring>

namespace krys::gfx
{
  namespace
  {
    // A unit quad drawn as a 4-vertex strip per instance; the corner comes from gl_VertexID.
    constexpr const char8_t *RectVertexSource = u8R"(#version 450
layout(binding = 0, std140) uniform Frame
{
  vec2 u_ViewportSize;
  uint u_DecodeSrgb;
};

layout(location = 0) in vec4 a_Rect;   // x, y, width, height in pixels
layout(location = 1) in vec4 a_Colour; // sRGB, straight alpha

layout(location = 0) flat out vec4 v_Colour;

vec3 SrgbToLinear(vec3 c)
{
  return mix(c / 12.92, pow((c + 0.055) / 1.055, vec3(2.4)), greaterThan(c, vec3(0.04045)));
}

void main()
{
  vec2 corner = vec2(gl_VertexID & 1, gl_VertexID >> 1);
  vec2 pixel = a_Rect.xy + corner * a_Rect.zw;
  vec2 ndc = pixel / u_ViewportSize * 2.0 - 1.0;
  gl_Position = vec4(ndc.x, -ndc.y, 0.0, 1.0);

  // sRGB targets blend in linear light and encode on write; other targets blend the encoded values.
  vec3 rgb = u_DecodeSrgb != 0u ? SrgbToLinear(a_Colour.rgb) : a_Colour.rgb;
  v_Colour = vec4(rgb * a_Colour.a, a_Colour.a);
}
)";

    constexpr const char8_t *RectFragmentSource = u8R"(#version 450
layout(location = 0) flat in vec4 v_Colour;
layout(location = 0) out vec4 o_Colour;

void main()
{
  o_Colour = v_Colour;
}
)";
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
    _rects.clear();
  }

  void Renderer2D::FillRect(const Rect2D &rect, SRGBA<uint8> colour) noexcept
  {
    krys_debug_assert_msg(_commands != nullptr, "FillRect outside Begin and End.");
    if (rect.Width <= 0.f || rect.Height <= 0.f)
    {
      return;
    }

    const auto resolved = colour.Resolved();
    if (resolved.Alpha == 0u)
    {
      return;
    }
    _rects.push_back({.Rect = {rect.X, rect.Y, rect.Width, rect.Height},
                      .Colour = {resolved.Red, resolved.Green, resolved.Blue, resolved.Alpha}});
  }

  void Renderer2D::End() noexcept
  {
    krys_assert_msg(_commands != nullptr, "End without Begin.");
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
        commands.Draw(4u, static_cast<uint32>(_rects.size()));
      }
    }

    commands.EndRenderPass();
    _rects.clear();
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
      .ColourFormats = {format},
      .DebugName = u8"Render2D.Rect",
    };
    desc.VertexBuffers.push_back({
      .Stride = sizeof(RectInstance),
      .InputRate = VertexInputRate::PerInstance,
      .Attributes = {{0u, VertexFormat::Float4, offsetof(RectInstance, Rect)},
                     {1u, VertexFormat::UByte4Norm, offsetof(RectInstance, Colour)}},
    });

    auto pipeline = _device->CreatePipeline(desc);
    if (!pipeline)
    {
      return Unexpected<string>(pipeline.error());
    }
    _pipelines.push_back({.Format = format, .Pipeline = *pipeline});
    return *pipeline;
  }
}
