#pragma once

#include "Krystal.Gfx.OpenGL/Context.hpp"
#include "Krystal.Gfx/IRenderer.hpp"
#include "Krystal.Lib/List.hpp"
#include "Krystal.Lib/Macros.hpp"
#include "Krystal.Lib/Types.hpp"

namespace Krys::Gfx::OpenGL
{
  struct UIElementInstanceData
  {
    Colour BackgroundColour;
    Colour BorderColour;
    Maths::Vec4 PositionAndSize;       // xy: position, zw: size
    Maths::Vec2 BorderThicknessRadius; // x: border thickness, y: border radius

    constexpr static VertexBufferLayout Layout()
    {
      return {
        {VertexAttributeType::Float, 4, VertexInputRate::PerInstance}, // background colour
        {VertexAttributeType::Float, 4, VertexInputRate::PerInstance}, // border colour
        {VertexAttributeType::Float, 4, VertexInputRate::PerInstance}, // quad position and size
        {VertexAttributeType::Float, 2, VertexInputRate::PerInstance}, // border thickness and radius
      };
    }
  };

  struct UIData
  {
    ShaderHandle Shader;
    GLuint Vao {0u};
    BufferHandle Vbo;
    BufferHandle Ebo;
    List<UIElementInstanceData> InstanceData;
    BufferHandle InstanceDataBuffer;
    RenderTargetHandle CurrentRenderTarget {0u};
  };

  class Renderer : public IRenderer
  {
    NO_COPY_MOVE(Renderer)

  private:
    Context &_context;
    UIData _uiData;

  public:
    Renderer(IContext &context) noexcept;

    ~Renderer() override = default;

    void Startup() override;

    void Shutdown() noexcept override;

    void BeginFrame() override;

    void EndFrame() override;

    void Submit(const CommandList &commandList) override;
  };
}