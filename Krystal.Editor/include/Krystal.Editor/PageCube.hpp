#pragma once

#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/DOM/EventListenerHandle.hpp"
#include "Krystal.Booey/DOM/PointerEvent.hpp"
#include "Krystal.Booey/HTML/Canvas/EngineTextureRenderingContext.hpp"
#include "Krystal.Booey/HTML/HTMLCanvasElement.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Maths/Clipspace.hpp"
#include "Krystal.Core/Maths/Transform.hpp"
#include "Krystal.Core/Types/Array.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"
#include "Krystal.Gfx/CommandBuffer.hpp"
#include "Krystal.Gfx/Device.hpp"
#include "Krystal.UI/EngineTexture.hpp"
#include <cmath>
#include <format>
#include <numbers>

namespace krys::editor
{
  /// @brief The editor page's live 3D view: a cube drawn with Krystal.Gfx into a texture each frame, shown by
  /// the `"krystal-texture"` contexts of `#cube` (which turns it when dragged) and `#cube-strip`. A page
  /// without `#cube` is left alone.
  class PageCube
  {
    KRYS_NON_COPY_MOVABLE(PageCube);

    struct Vertex
    {
      float Position[3];
      float Colour[3];
      float Normal[3];
    };

    static constexpr uint32 TextureSize = 480u;
    /// @brief Radians a second the cube turns while it isn't dragged.
    static constexpr float IdleSpeed = 0.6f;
    /// @brief Radians the cube turns per CSS pixel dragged.
    static constexpr float DragSpeed = 0.012f;

    static constexpr utf8_stringview VertexSource = u8R"(#version 450
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColour;
layout(location = 2) in vec3 inNormal;
layout(binding = 0, std140) uniform Transform { mat4 ModelViewProjection; mat4 Model; };
layout(location = 0) out vec3 colour;
layout(location = 1) out vec3 normal;
void main()
{
  gl_Position = ModelViewProjection * vec4(inPosition, 1.0);
  colour = inColour;
  normal = mat3(Model) * inNormal;
}
)";

    static constexpr utf8_stringview FragmentSource = u8R"(#version 450
layout(location = 0) in vec3 colour;
layout(location = 1) in vec3 normal;
layout(location = 0) out vec4 outColour;
void main()
{
  // Lit from above and in front, faces turned away kept at three quarters.
  float light = 0.75 + 0.25 * max(dot(normalize(normal), normalize(vec3(0.4, 0.7, 1.0))), 0.0);
  outColour = vec4(colour * light, 1.0);
}
)";

  private:
    gfx::Device &_device;
    WeakPtr<boo::dom::Document> _document;
    List<boo::dom::EventListenerHandle> _listeners;
    List<boo::html::EngineTextureRenderingContext *> _contexts;
    gfx::TextureHandle _texture {};
    gfx::TextureHandle _depth {};
    gfx::ShaderHandle _vertexShader {};
    gfx::ShaderHandle _fragmentShader {};
    gfx::PipelineHandle _pipeline {};
    gfx::BufferHandle _vertices {};
    gfx::BufferHandle _indices {};
    gfx::BufferHandle _transform {};
    float _yaw {0.6f};
    float _pitch {-0.45f};
    int32 _dragPointer {-1};
    double _lastX {0};
    double _lastY {0};

  public:
    PageCube(boo::dom::Document &document, gfx::Device &device) noexcept
        : _device(device), _document(WeakPtr<boo::dom::Document>::WithRef(document))
    {
      auto cube = document.GetElementById(u8"cube");
      if (!cube || !CreateResources())
      {
        return;
      }

      auto texture = ui::EngineTexture::Create(
        _texture, {static_cast<int32>(TextureSize), static_cast<int32>(TextureSize)});
      for (auto id : {u8"cube", u8"cube-strip"})
      {
        auto *canvas = DynamicDowncast<boo::html::HTMLCanvasElement>(document.GetElementById(id).get());
        if (!canvas)
        {
          continue;
        }
        auto context = canvas->GetContext(boo::html::EngineTextureRenderingContext::ContextType);
        if (auto *textureContext = DynamicDowncast<boo::html::EngineTextureRenderingContext>(context.get()))
        {
          textureContext->SetTexture(texture);
          _contexts.push_back(textureContext);
        }
      }

      // The pointer that presses the cube captures itself to it, and turns the cube until it lets go.
      auto listen = [&](utf8_stringview type, Func<void(boo::dom::PointerEvent &)> &&callback)
      {
        _listeners.push_back(cube->AddEventListener(
          boo::dom::DOMStringAtom(type), [callback = krys::move(callback)](boo::dom::Event &event)
          { callback(static_cast<boo::dom::PointerEvent &>(event)); }));
      };
      listen(u8"pointerdown",
             [this](boo::dom::PointerEvent &pointer)
             {
               if (auto element = ById(u8"cube"))
               {
                 _lastX = pointer.ClientX();
                 _lastY = pointer.ClientY();
                 (void)element->SetPointerCapture(static_cast<int32>(pointer.PointerId()));
               }
             });
      listen(u8"gotpointercapture", [this](boo::dom::PointerEvent &pointer)
             { _dragPointer = static_cast<int32>(pointer.PointerId()); });
      listen(u8"lostpointercapture", [this](boo::dom::PointerEvent &) { _dragPointer = -1; });
      listen(u8"pointermove", [this](boo::dom::PointerEvent &pointer) { Drag(pointer); });
    }

    ~PageCube() noexcept
    {
      // The canvases show nothing once the texture is gone.
      if (_document)
      {
        for (auto *context : _contexts)
        {
          context->SetTexture(nullptr);
        }
      }
      for (auto buffer : {_vertices, _indices, _transform})
      {
        if (buffer.IsValid())
        {
          _device.Destroy(buffer);
        }
      }
      if (_pipeline.IsValid())
      {
        _device.Destroy(_pipeline);
      }
      for (auto shader : {_vertexShader, _fragmentShader})
      {
        if (shader.IsValid())
        {
          _device.Destroy(shader);
        }
      }
      for (auto texture : {_texture, _depth})
      {
        if (texture.IsValid())
        {
          _device.Destroy(texture);
        }
      }
    }

    /// @brief Turns the cube by itself while it isn't dragged.
    void Update(float seconds) noexcept
    {
      if (_dragPointer < 0)
      {
        _yaw += IdleSpeed * seconds;
      }
    }

    /// @brief Draws the cube into its texture with @p commands, ahead of the page, and tells the canvases.
    void Render(gfx::CommandBuffer &commands) noexcept
    {
      if (_contexts.empty() || !_pipeline.IsValid())
      {
        return;
      }

      auto model = Rotate(Rotate(Identity<Mat4>(), _pitch, Vec3(1.f, 0.f, 0.f)), _yaw, Vec3(0.f, 1.f, 0.f));
      auto view = Translate(Vec3(0.f, 0.f, -3.4f));
      auto projection = Perspective_RH_ZO(std::numbers::pi_v<float> / 4.f, 1.f, 0.1f, 10.f);
      Array<Mat4, 2> transform {projection * view * model, model};
      _device.WriteBuffer(_transform, 0u,
                          {reinterpret_cast<const byte *>(transform.data()), sizeof(transform)});

      auto pass = gfx::RenderPassDesc::SingleColour(
        {.Texture = _texture, .Load = gfx::AttachmentLoadOp::Clear, .ClearColour = {0.f, 0.f, 0.f, 0.f}});
      pass.DepthStencil = {.Texture = _depth};
      pass.DebugName = u8"Editor.Cube";
      commands.BeginRenderPass(pass);
      commands.BindPipeline(_pipeline);
      commands.BindVertexBuffer(0u, _vertices);
      commands.BindIndexBuffer(_indices, gfx::IndexType::UInt16);
      commands.BindUniformBuffer(0u, _transform, 0u, sizeof(transform));
      commands.DrawIndexed(36u);
      commands.EndRenderPass();

      for (auto *context : _contexts)
      {
        context->TextureChanged();
      }
    }

  private:
    KRYS_NODISCARD bool CreateResources() noexcept
    {
      auto texture =
        _device.CreateTexture({.Format = gfx::TextureFormat::RGBA8Unorm,
                               .Width = TextureSize,
                               .Height = TextureSize,
                               .Usage = gfx::TextureUsage::RenderTarget | gfx::TextureUsage::Sampled,
                               .DebugName = u8"Editor.Cube"});
      auto depth = _device.CreateTexture({.Format = gfx::TextureFormat::Depth32Float,
                                          .Width = TextureSize,
                                          .Height = TextureSize,
                                          .Usage = gfx::TextureUsage::RenderTarget,
                                          .DebugName = u8"Editor.CubeDepth"});
      auto vertexShader = _device.CreateShader(
        {.Stage = gfx::ShaderStage::Vertex, .Source = VertexSource, .DebugName = u8"Editor.Cube"});
      auto fragmentShader = _device.CreateShader(
        {.Stage = gfx::ShaderStage::Fragment, .Source = FragmentSource, .DebugName = u8"Editor.Cube"});
      if (!texture || !depth || !vertexShader || !fragmentShader)
      {
        return false;
      }
      _texture = *texture;
      _depth = *depth;
      _vertexShader = *vertexShader;
      _fragmentShader = *fragmentShader;

      gfx::PipelineDesc pipeline {.VertexShader = _vertexShader,
                                  .FragmentShader = _fragmentShader,
                                  .DepthStencil = {.DepthTest = true, .DepthWrite = true},
                                  .Raster = {.Cull = gfx::CullMode::Back},
                                  .DepthStencilFormat = gfx::TextureFormat::Depth32Float,
                                  .DebugName = u8"Editor.Cube"};
      pipeline.VertexBuffers.push_back(
        {.Stride = sizeof(Vertex),
         .Attributes = {
           {.Location = 0u, .Format = gfx::VertexFormat::Float3, .Offset = 0u},
           {.Location = 1u, .Format = gfx::VertexFormat::Float3, .Offset = 3u * sizeof(float)},
           {.Location = 2u, .Format = gfx::VertexFormat::Float3, .Offset = 6u * sizeof(float)}}});
      pipeline.ColourFormats.push_back(gfx::TextureFormat::RGBA8Unorm);
      auto created = _device.CreatePipeline(pipeline);
      if (!created)
      {
        return false;
      }
      _pipeline = *created;

      // Each face in a colour of the page's palette, its corners counter-clockwise seen from outside.
      constexpr Array<Array<float, 3>, 6> colours {{{0.937f, 0.278f, 0.435f},
                                                    {1.0f, 0.820f, 0.400f},
                                                    {0.024f, 0.839f, 0.627f},
                                                    {0.067f, 0.541f, 0.698f},
                                                    {0.969f, 0.549f, 0.420f},
                                                    {0.545f, 0.769f, 0.894f}}};
      // The faces' axes: the normal, then two edges with right-handed order.
      constexpr Array<Array<Array<float, 3>, 3>, 6> faces {{{{{1, 0, 0}, {0, 0, -1}, {0, 1, 0}}},
                                                            {{{-1, 0, 0}, {0, 0, 1}, {0, 1, 0}}},
                                                            {{{0, 1, 0}, {1, 0, 0}, {0, 0, -1}}},
                                                            {{{0, -1, 0}, {1, 0, 0}, {0, 0, 1}}},
                                                            {{{0, 0, 1}, {1, 0, 0}, {0, 1, 0}}},
                                                            {{{0, 0, -1}, {-1, 0, 0}, {0, 1, 0}}}}};
      List<Vertex> vertices;
      List<uint16> indices;
      for (size_t face = 0; face < faces.size(); ++face)
      {
        const auto &[normal, u, v] = faces[face];
        const auto first = static_cast<uint16>(vertices.size());
        for (auto [su, sv] : {Array<float, 2> {-1, -1}, {1, -1}, {1, 1}, {-1, 1}})
        {
          Vertex vertex {};
          for (size_t axis = 0; axis < 3; ++axis)
          {
            vertex.Position[axis] = 0.6f * (normal[axis] + su * u[axis] + sv * v[axis]);
            vertex.Colour[axis] = colours[face][axis];
            vertex.Normal[axis] = normal[axis];
          }
          vertices.push_back(vertex);
        }
        for (uint16 index : Array<uint16, 6> {0, 1, 2, 0, 2, 3})
        {
          indices.push_back(static_cast<uint16>(first + index));
        }
      }

      auto vertexBuffer = _device.CreateBuffer(
        {.Size = vertices.size() * sizeof(Vertex),
         .Usage = gfx::BufferUsage::Vertex,
         .InitialData = {reinterpret_cast<const byte *>(vertices.data()), vertices.size() * sizeof(Vertex)},
         .DebugName = u8"Editor.Cube"});
      auto indexBuffer = _device.CreateBuffer(
        {.Size = indices.size() * sizeof(uint16),
         .Usage = gfx::BufferUsage::Index,
         .InitialData = {reinterpret_cast<const byte *>(indices.data()), indices.size() * sizeof(uint16)},
         .DebugName = u8"Editor.Cube"});
      auto transformBuffer = _device.CreateBuffer({.Size = 2u * sizeof(Mat4),
                                                   .Usage = gfx::BufferUsage::Uniform,
                                                   .Memory = gfx::MemoryUsage::Dynamic,
                                                   .DebugName = u8"Editor.Cube"});
      if (!vertexBuffer || !indexBuffer || !transformBuffer)
      {
        return false;
      }
      _vertices = *vertexBuffer;
      _indices = *indexBuffer;
      _transform = *transformBuffer;
      return true;
    }

    KRYS_NODISCARD RefPtr<boo::dom::Element> ById(utf8_stringview id) const noexcept
    {
      return _document ? _document->GetElementById(id) : nullptr;
    }

    void Drag(const boo::dom::PointerEvent &pointer) noexcept
    {
      if (static_cast<int32>(pointer.PointerId()) != _dragPointer)
      {
        return;
      }
      _yaw += static_cast<float>(pointer.ClientX() - _lastX) * DragSpeed;
      _pitch = std::clamp(_pitch + static_cast<float>(pointer.ClientY() - _lastY) * DragSpeed, -1.5f, 1.5f);
      _lastX = pointer.ClientX();
      _lastY = pointer.ClientY();

      if (auto label = ById(u8"cube-angle"))
      {
        auto degrees = [](float radians)
        {
          return std::lround(std::remainder(radians, 2.f * std::numbers::pi_v<float>) * 180.f
                             / std::numbers::pi_v<float>);
        };
        auto text = std::format("Turned {}° across, {}° up", degrees(_yaw), -degrees(_pitch));
        (void)label->TextContent(
          boo::dom::DOMString(reinterpret_cast<const char8_t *>(text.data()), text.size()));
      }
    }
  };
}
