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

    constexpr static uint32 BatchSize = 200u;

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

  struct GlyphVertex
  {
    Maths::Vec2 Position {};
    Maths::Vec2 UV {};

    constexpr static uint32 BatchSize = 4'096;
    constexpr static uint32 VerticesPerGlyph = 6; // 2 triangles per glyph

    static VertexBufferLayout Layout() noexcept
    {
      return {
        {VertexAttributeType::Float, 2}, // Position
        {VertexAttributeType::Float, 2}  // UV
      };
    }
  };

  class Renderer : public IRenderer
  {
    NO_COPY_MOVE(Renderer)

  private:
    Context &_context;
    ShaderHandle _quadShader;
    ShaderHandle _singleTextureShader;
    MeshHandle _quadMesh;
    RenderTargetHandle _currentRenderTarget;
    InstanceData<QuadInstanceData> _quadInstanceData;
    List<GlyphVertex> _glyphVertices {};
    BufferHandle _glyphBuffer {};
    GLuint _textVao {};
    int _dpi;

  public:
    Renderer(IContext &context) noexcept;

    ~Renderer() override = default;

    void Startup() override;

    void Shutdown() noexcept override;

    void BeginFrame() override;

    void EndFrame() override;

    void Submit(const CommandList &commandList) override;

    void DPIChanged(int dpi) noexcept override;

  private:
    void FlushQuadInstances();

    void DrawTexturedQuad(GLuint texture, const Maths::Vec2 &position, const Maths::Vec2 &size,
                          float opacity);

    void DrawText(const string &text, FontHandle fontHandle, float ptSize, const Maths::Vec2 &position,
                  const Colour &colour = Colours::Black) noexcept;

    void DrawTextOutlined(const string &text, FontHandle fontHandle, float ptSize,
                          const Maths::Vec2 &position,
                          const Colour &textColour = Colours::Black,
                          const Colour &outlineColour = Colours::White, float outlineWidth = 3.f) noexcept;

    void DrawText(Font &font, Shader &shader, const string &text, const Colour &textColour,
                  const Maths::Vec2 &position, float ptSize);
  };
}