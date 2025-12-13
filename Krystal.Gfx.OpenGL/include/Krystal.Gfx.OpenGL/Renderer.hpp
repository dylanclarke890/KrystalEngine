#pragma once

#include "Krystal.Gfx.OpenGL/Context.hpp"
#include "Krystal.Gfx/Commands/CommandList.hpp"
#include "Krystal.Gfx/Handle.hpp"
#include "Krystal.Gfx/InstanceData.hpp"
#include "Krystal.Gfx/IRenderer.hpp"
#include "Krystal.Lib/List.hpp"
#include "Krystal.Lib/Macros.hpp"
#include "Krystal.Lib/Types.hpp"

namespace Krys::Gfx::OpenGL
{
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
    RenderTargetHandle _currentRenderTarget;
    List<GlyphVertex> _glyphVertices {};
    BufferHandle _glyphBuffer {};
    GLuint _textVao {};

  public:
    Renderer(IContext &context) noexcept;

    ~Renderer() override = default;

    void Startup() override;

    void Shutdown() noexcept override;

    void BeginFrame() override;

    void EndFrame() override;

    void Submit(const CommandList &commandList) override;

  private:
    void DrawText(Font &font, Shader &shader, const utf8_string &text, const ColourbPremultiplied &textColour,
                  const Maths::Vec2 &position, float ptSize);

    void DrawTextOutlined(const utf8_string &text, FontHandle fontHandle, float ptSize,
                          const Maths::Vec2 &position,
                          const ColourbPremultiplied &textColour = Colours::Black,
                          const ColourbPremultiplied &outlineColour = Colours::White,
                          float outlineWidth = 3.f) noexcept;
  };
}