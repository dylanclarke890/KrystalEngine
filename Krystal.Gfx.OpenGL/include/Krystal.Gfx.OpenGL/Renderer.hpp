#pragma once

#include "Krystal.Gfx.OpenGL/Context.hpp"
#include "Krystal.Gfx/Handle.hpp"
#include "Krystal.Gfx/InstanceData.hpp"
#include "Krystal.Gfx/IRenderer.hpp"
#include "Krystal.Lib/List.hpp"
#include "Krystal.Lib/Macros.hpp"
#include "Krystal.Lib/Types.hpp"

namespace Krys::Gfx::OpenGL
{
  struct QuadInstanceData
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

    constexpr static size_t BatchSize = 200u;
  };

  class Renderer : public IRenderer
  {
    NO_COPY_MOVE(Renderer)

  private:
    Context &_context;
    TextRenderer _textRenderer;
    ShaderHandle _quadShader;
    ShaderHandle _singleTextureShader;
    MeshHandle _quadMesh;
    RenderTargetHandle _currentRenderTarget;
    InstanceData<QuadInstanceData> _quadInstanceData;

  public:
    Renderer(IContext &context) noexcept;

    ~Renderer() override = default;

    void Startup() override;

    void Shutdown() noexcept override;

    void BeginFrame() override;

    void EndFrame() override;

    void Submit(const CommandList &commandList) override;

  private:
    void FlushQuadInstances();
    void DrawTexturedQuad(GLuint texture, const Maths::Vec2 &position, const Maths::Vec2 &size, float opacity);
  };
}