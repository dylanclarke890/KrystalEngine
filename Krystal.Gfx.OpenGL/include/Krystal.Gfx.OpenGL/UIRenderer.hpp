#pragma once

#include "Krystal.Gfx.OpenGL/Context.hpp"
#include "Krystal.Gfx.OpenGL/gl.hpp"
#include "Krystal.Gfx.OpenGL/Registries/BufferRegistry.hpp"
#include "Krystal.Gfx.OpenGL/Registries/ShaderRegistry.hpp"
#include "Krystal.Gfx/IUIRenderer.hpp"
#include "Krystal.Lib/List.hpp"
#include "Krystal.Lib/Macros.hpp"

namespace Krys::Gfx::OpenGL
{
  class UIRenderer : public IUIRenderer
  {
    NO_COPY_MOVE(UIRenderer)

  private:
    Context &_context;
    List<UIElementVertex> _vertices;
    List<UIElementUboData> _uboData;
    ShaderHandle _shader;
    GLuint _vao {0u};
    BufferHandle _vbo;
    BufferHandle _ebo;
    BufferHandle _ubo;
    size_t _index {0u};

  public:
    UIRenderer(IContext &ctx) noexcept;
    ~UIRenderer() noexcept override = default;

    void Startup() override
    {
      _shader = _context.Shaders().Load(IO::Path("ui.vert"), IO::Path("ui.frag"));

      auto &buffers = static_cast<BufferRegistry &>(_context.Buffers());

      const size_t quadVertexCount = 4;
      const size_t quadIndexCount = 6;
      const size_t batchSize = 1'000;

      _vbo = buffers.CreateVertexBuffer(quadVertexCount * batchSize * sizeof(UIElementVertex));
      _ebo = buffers.CreateIndexBuffer(quadIndexCount * batchSize * sizeof(uint32));
      _ubo = buffers.CreateUniformBuffer(batchSize * sizeof(UIElementUboData));

      auto &vbo = buffers.Get(_vbo);
      auto &ebo = buffers.Get(_ebo);
      auto &ubo = buffers.Get(_ubo);

      glCreateVertexArrays(1, &_vao);
      glBindVertexArray(_vao);
      vbo.Bind();
      Utils::ApplyVertexBufferLayout(UIElementVertex::Layout());
      ebo.Bind();
      ubo.Bind(5); // TODO: get from config/shader

      // Precompute index buffer data
      List<uint32> indices;
      indices.reserve(quadIndexCount * batchSize);
      for (uint32 i = 0; i < batchSize; ++i)
      {
        uint32 offset = i * quadVertexCount;
        indices.push_back(offset + 0);
        indices.push_back(offset + 1);
        indices.push_back(offset + 2);
        indices.push_back(offset + 2);
        indices.push_back(offset + 3);
        indices.push_back(offset + 0);
      }
      ebo.Update(indices);

      _vertices.reserve(quadVertexCount * batchSize);
      _uboData.reserve(batchSize);

      glObjectLabel(GL_BUFFER, vbo.GetHandle(), -1, "UI-VBO");
      glObjectLabel(GL_BUFFER, ebo.GetHandle(), -1, "UI-EBO");
      glObjectLabel(GL_BUFFER, ubo.GetHandle(), -1, "UI-UBO");
    }

    void Shutdown() noexcept override
    {
      _context.Shaders().Unload(_shader);
      _context.Buffers().Destroy(_vbo);
      _context.Buffers().Destroy(_ebo);
      _context.Buffers().Destroy(_ubo);
      glDeleteVertexArrays(1, &_vao);
    }

    void BeginFrame() override
    {
      _index = 0u;
      _vertices.clear();
      _uboData.clear();
    }

    void EndFrame() override
    {
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      {
        auto &vbo = static_cast<BufferRegistry &>(_context.Buffers()).Get(_vbo);
        auto &ubo = static_cast<BufferRegistry &>(_context.Buffers()).Get(_ubo);

        auto &shader = static_cast<ShaderRegistry &>(_context.Shaders()).Get(_shader);
        shader.Bind();

        vbo.Update(_vertices);
        ubo.Update(_uboData);

        glBindVertexArray(_vao);
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(_index * 6), GL_UNSIGNED_INT, nullptr);
      }

      glDisable(GL_BLEND);
    }

    void Render(UI::Document &document, float width, float height) override
    {
      UIRenderer::BeginFrame();
      {
        document.Layout(width, height);
        DrawElement(document.GetBody(), document, height);
      }
      UIRenderer::EndFrame();
    }

  private:
    void DrawElement(UI::Element &element, UI::Document &document, float height)
    {
      UI::ComputedBounds cb = element.GetComputedBounds();
      cb.Y = static_cast<float>(height) - cb.Y - cb.Height;
      Gfx::Colour col = element.GetBackgroundColor();

      _vertices.push_back({Maths::Vec2(cb.MinX(), cb.MinY()), Maths::Vec2(0.0f, 0.0f), col});
      _vertices.push_back({Maths::Vec2(cb.MaxX(), cb.MinY()), Maths::Vec2(1.0f, 0.0f), col});
      _vertices.push_back({Maths::Vec2(cb.MaxX(), cb.MaxY()), Maths::Vec2(1.0f, 1.0f), col});
      _vertices.push_back({Maths::Vec2(cb.MinX(), cb.MaxY()), Maths::Vec2(0.0f, 1.0f), col});

      _uboData.push_back(
        {.RectSize = {cb.Width, cb.Height}, .BorderThicknessRadius = {0.f, 0.f}, .FillColour = col});

      _index++;

      for (const auto &childHandle : element.GetChildren())
      {
        auto &childElement = document.GetByHandle(childHandle);
        DrawElement(childElement, document, height);
      }
    }
  };
}