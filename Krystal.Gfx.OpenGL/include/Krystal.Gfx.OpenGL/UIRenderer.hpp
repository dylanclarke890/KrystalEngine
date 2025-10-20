#pragma once

#include "Krystal.Gfx.OpenGL/Context.hpp"
#include "Krystal.Gfx.OpenGL/gl.hpp"
#include "Krystal.Gfx.OpenGL/Registries/BufferRegistry.hpp"
#include "Krystal.Gfx.OpenGL/Registries/ShaderRegistry.hpp"
#include "Krystal.Gfx.OpenGL/Utils.hpp"
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
    List<UIElementInstanceData> _instanceData;
    ShaderHandle _shader;
    GLuint _vao {0u};
    BufferHandle _vbo;
    BufferHandle _ebo;
    BufferHandle _instanceDataBuffer;

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

      // Create VAO
      glCreateVertexArrays(1, &_vao);
      glBindVertexArray(_vao);

      // Setup indices
      _ebo = buffers.CreateIndexBuffer(quadIndexCount * sizeof(uint32));
      auto &ebo = buffers.Get(_ebo);
      ebo.Bind();
      Array<uint32, 6> indices {0, 1, 2, 2, 3, 0};
      ebo.Update(indices);

      // Setup vertices
      _vbo = buffers.CreateVertexBuffer(quadVertexCount * sizeof(UIElementVertex));
      auto &vbo = buffers.Get(_vbo);
      vbo.Bind();
      Array<UIElementVertex, quadVertexCount> vertices = {
        UIElementVertex {{0.0f, 0.0f}, {0.0f, 0.0f}},
        UIElementVertex {{1.0f, 0.0f}, {1.0f, 0.0f}},
        UIElementVertex {{1.0f, 1.0f}, {1.0f, 1.0f}},
        UIElementVertex {{0.0f, 1.0f}, {0.0f, 1.0f}},
      };
      vbo.Update(vertices);
      Utils::ApplyVertexBufferLayout(UIElementVertex::Layout());

      // Setup instance data
      _instanceDataBuffer = buffers.CreateVertexBuffer(batchSize * sizeof(UIElementInstanceData));
      auto &instanceDataBuffer = buffers.Get(_instanceDataBuffer);
      instanceDataBuffer.Bind();
      const size_t instanceDataAttributeOffset = UIElementVertex::Layout().size();
      Utils::ApplyVertexBufferLayout(UIElementInstanceData::Layout(), instanceDataAttributeOffset);
      _instanceData.reserve(batchSize); // Nothing to update yet, but allocate enough space

      glObjectLabel(GL_VERTEX_ARRAY, _vao, -1, "UI_VAO");
      glObjectLabel(GL_BUFFER, vbo.GetHandle(), -1, "UI_VBO");
      glObjectLabel(GL_BUFFER, ebo.GetHandle(), -1, "UI_EBO");
      glObjectLabel(GL_BUFFER, instanceDataBuffer.GetHandle(), -1, "UI_InstanceData");
    }

    void Shutdown() noexcept override
    {
      _context.Shaders().Unload(_shader);
      _context.Buffers().Destroy(_vbo);
      _context.Buffers().Destroy(_ebo);
      _context.Buffers().Destroy(_instanceDataBuffer);
      glDeleteVertexArrays(1, &_vao);
    }

    void BeginFrame() override
    {
      _instanceData.clear();
    }

    void EndFrame() override
    {
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      {
        auto &shader = static_cast<ShaderRegistry &>(_context.Shaders()).Get(_shader);
        shader.Bind();

        auto &buffer = static_cast<BufferRegistry &>(_context.Buffers()).Get(_instanceDataBuffer);
        buffer.Update(_instanceData);

        glBindVertexArray(_vao);
        uint32 instanceCount = static_cast<GLsizei>(_instanceData.size());

        glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr, instanceCount);
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

      _instanceData.push_back({
        .BackgroundColour = element.GetBackgroundColor(),
        .BorderColour = element.GetBorderColor(),
        .PositionAndSize = {cb.X, cb.Y, cb.Width, cb.Height},
        .BorderThicknessRadius = {element.GetBorderWidth(), element.GetBorderRadius()},
      });

      for (const auto &childHandle : element.GetChildren())
      {
        auto &childElement = document.GetByHandle(childHandle);
        DrawElement(childElement, document, height);
      }
    }
  };
}