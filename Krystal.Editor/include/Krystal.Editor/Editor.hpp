#pragma once

#ifndef KRYS_ENABLE_PROFILING
  #define KRYS_ENABLE_PROFILING
#endif

#include "Krystal.Core/Log/ILogger.hpp"
#include "Krystal.Core/Types/Array.hpp"
#include "Krystal.Core/Utils/ByteUtils.hpp"
#include "Krystal.Core/Utils/ScopedProfiler.hpp"
#include "Krystal.Editor/Application.hpp"
#include "Krystal.PAL/Events.hpp"
#include "Krystal.PAL/Keys.hpp"
#include <cstddef>

namespace krys
{
  class Editor : public Application
  {
    struct TriangleVertex
    {
      float Position[2];
      uint8 Colour[4];
    };

    struct TriangleUniforms
    {
      float Time;
      float Padding[3];
    };

    constexpr static const char8_t *TriangleVertexSource = u8R"(#version 450
layout(location = 0) in vec2 a_Position;
layout(location = 1) in vec4 a_Colour;

KRYS_PUSH_CONSTANTS Draw
{
  float u_Time;
};

layout(location = 0) out vec4 v_Colour;

void main()
{
  float angle = u_Time * 0.5;
  mat2 rotation = mat2(cos(angle), sin(angle), -sin(angle), cos(angle));
  gl_Position = vec4(rotation * a_Position, 0.0, 1.0);
  v_Colour = a_Colour;
}
)";

    constexpr static const char8_t *TriangleFragmentSource = u8R"(#version 450
layout(location = 0) in vec4 v_Colour;
layout(location = 0) out vec4 o_Colour;

void main()
{
  o_Colour = v_Colour;
}
)";

  private:
    uint32 _width {0u};
    uint32 _height {0u};
    float _time {0.f};
    gfx::ShaderHandle _vertexShader {};
    gfx::ShaderHandle _fragmentShader {};
    gfx::PipelineHandle _pipeline {};
    gfx::BufferHandle _vertices {};

  public:
    explicit Editor(int argc, char **argv, const ApplicationSettings &settings) noexcept
        : Application(argc, argv, settings), _width(settings.WindowSettings.Size.Width),
          _height(settings.WindowSettings.Size.Height)
    {
    }

    void OnInit() noexcept override
    {
      _context->Logger->SetLevel(log::Level::Info);
      _context->Logger->Info("Initialising Krystal Editor...");
      BindEvents();
      CreateTriangle();
    }

    void OnShutdown() noexcept override
    {
      auto &device = *_context->Device;
      if (_vertices.IsValid())
      {
        device.Destroy(_vertices);
      }
      if (_pipeline.IsValid())
      {
        device.Destroy(_pipeline);
      }
      if (_fragmentShader.IsValid())
      {
        device.Destroy(_fragmentShader);
      }
      if (_vertexShader.IsValid())
      {
        device.Destroy(_vertexShader);
      }
    }

    void BindEvents() noexcept
    {
      using namespace pal;

      _context->Events->On<KeyboardEvent>(
        [&](const auto &event)
        {
          if (event.State() == pal::KeyState::Pressed)
          {
            _context->Logger->Info("Key Pressed '{}'", event.Key());
          }

          if (event.Key() == pal::Key::Escape)
          {
            _context->Logger->Info("ESC key pressed, quitting application...");
            Stop();
          }
          return true;
        });

      _context->Events->On<WindowResizeEvent>(
        [&](const auto &event)
        {
          _width = event.Width();
          _height = event.Height();
          return true;
        });

      _context->Events->On<WindowCloseEvent>(
        [&](const auto &)
        {
          Stop();
          return true;
        });
    }

    void OnUpdate(seconds dt) noexcept override
    {
      _time += static_cast<float>(dt.count());
    }

    void OnRender(gfx::CommandBuffer &commands) noexcept override
    {
      if (!_pipeline.IsValid() || _width == 0u || _height == 0u)
      {
        return;
      }

      commands.BeginRenderPass(gfx::RenderPassDesc::SingleColour(
        {.Swapchain = _context->Swapchain, .ClearColour = {0.02f, 0.02f, 0.03f, 1.f}}));
      commands.BindPipeline(_pipeline);
      commands.BindVertexBuffer(0u, _vertices);
      commands.PushConstants(TriangleUniforms {.Time = _time, .Padding = {}});
      commands.Draw(3u);
      commands.EndRenderPass();
    }

  private:
    void CreateTriangle() noexcept
    {
      auto &device = *_context->Device;
      auto vertexShader = device.CreateShader(
        {.Stage = gfx::ShaderStage::Vertex, .Source = TriangleVertexSource, .DebugName = u8"Triangle.vert"});
      auto fragmentShader = device.CreateShader({.Stage = gfx::ShaderStage::Fragment,
                                                 .Source = TriangleFragmentSource,
                                                 .DebugName = u8"Triangle.frag"});
      if (!vertexShader || !fragmentShader)
      {
        _context->Logger->Error("{}", !vertexShader ? vertexShader.error() : fragmentShader.error());
        return;
      }
      _vertexShader = *vertexShader;
      _fragmentShader = *fragmentShader;

      gfx::PipelineDesc pipelineDesc {
        .VertexShader = _vertexShader,
        .FragmentShader = _fragmentShader,
        .Blend = gfx::BlendState::PremultipliedAlpha(),
        .ColourFormats = {gfx::TextureFormat::RGBA8Srgb},
        .PushConstantSize = sizeof(TriangleUniforms),
        .DebugName = u8"Triangle",
      };
      pipelineDesc.VertexBuffers.push_back({
        .Stride = sizeof(TriangleVertex),
        .Attributes = {{0u, gfx::VertexFormat::Float2, offsetof(TriangleVertex, Position)},
                       {1u, gfx::VertexFormat::UByte4Norm, offsetof(TriangleVertex, Colour)}},
      });
      auto pipeline = device.CreatePipeline(pipelineDesc);
      if (!pipeline)
      {
        _context->Logger->Error("{}", pipeline.error());
        return;
      }
      _pipeline = *pipeline;

      constexpr Array<TriangleVertex, 3> vertices {{
        {{0.f, 0.6f}, {255, 64, 64, 255}},
        {{-0.6f, -0.45f}, {64, 255, 64, 255}},
        {{0.6f, -0.45f}, {64, 96, 255, 255}},
      }};
      auto buffer = device.CreateBuffer({.Size = sizeof(vertices),
                                         .Usage = gfx::BufferUsage::Vertex,
                                         .InitialData = ByteUtils::AsBytesView(vertices),
                                         .DebugName = u8"TriangleVertices"});
      if (!buffer)
      {
        _context->Logger->Error("{}", buffer.error());
        return;
      }
      _vertices = *buffer;
    }
  };
}
