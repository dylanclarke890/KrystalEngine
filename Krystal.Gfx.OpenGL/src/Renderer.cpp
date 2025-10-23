#include "Krystal.Gfx.OpenGL/Renderer.hpp"
#include "Krystal.Gfx/Commands/CommandListReader.hpp"
#include "Krystal.Gfx/Commands/Commands.hpp"
#include "Krystal.Gfx/Vertex.hpp"
#include "Krystal.Lib/Expected.hpp"

namespace Krys::Gfx
{
  Expected<Unique<IRenderer>> CreateRenderer(IContext &context) noexcept
  {
    try
    {
      return CreateUnique<OpenGL::Renderer>(context);
    }
    catch (const std::exception &e)
    {
      return Unexpected("Exception during OpenGL renderer creation: " + string(e.what()));
    }
  }
}

namespace Krys::Gfx::OpenGL
{
  Renderer::Renderer(IContext &context) noexcept : _context(static_cast<Context &>(context))
  {
  }

  void Renderer::Startup()
  {
    _uiData.Shader = _context.Shaders().Load(IO::Path("ui.vert"), IO::Path("ui.frag"));

    auto &buffers = static_cast<BufferRegistry &>(_context.Buffers());
    const size_t quadVertexCount = 4;
    const size_t quadIndexCount = 6;
    const size_t batchSize = 1'000;

    // Create VAO
    glCreateVertexArrays(1, &_uiData.Vao);
    glBindVertexArray(_uiData.Vao);

    // Setup indices
    _uiData.Ebo = buffers.Create({
      .Type = BufferType::Index,
      .Usage = BufferUsage::Static,
      .Size = quadIndexCount * sizeof(uint32),
    });
    auto &ebo = buffers.Get(_uiData.Ebo);
    ebo.Bind();
    Array<uint32, 6> indices {0, 1, 2, 2, 3, 0};
    ebo.Update(indices);

    using UIElementVertex = Vertex::Position2DUV;

    // Setup vertices
    _uiData.Vbo = buffers.Create({
      .Type = BufferType::Vertex,
      .Usage = BufferUsage::Dynamic,
      .Size = quadVertexCount * sizeof(UIElementVertex),
    });
    auto &vbo = buffers.Get(_uiData.Vbo);
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
    _uiData.InstanceDataBuffer = buffers.Create({
      .Type = BufferType::Vertex,
      .Usage = BufferUsage::Dynamic,
      .Size = batchSize * sizeof(UIElementInstanceData),
    });
    auto &instanceDataBuffer = buffers.Get(_uiData.InstanceDataBuffer);
    instanceDataBuffer.Bind();
    const size_t instanceDataAttributeOffset = UIElementVertex::Layout().size();
    Utils::ApplyVertexBufferLayout(UIElementInstanceData::Layout(), instanceDataAttributeOffset);
    _uiData.InstanceData.reserve(batchSize); // Nothing to update yet, but allocate enough space

    glObjectLabel(GL_VERTEX_ARRAY, _uiData.Vao, -1, "UI_VAO");
    glObjectLabel(GL_BUFFER, vbo.GetHandle(), -1, "UI_VBO");
    glObjectLabel(GL_BUFFER, ebo.GetHandle(), -1, "UI_EBO");
    glObjectLabel(GL_BUFFER, instanceDataBuffer.GetHandle(), -1, "UI_InstanceData");
  }

  void Renderer::Shutdown() noexcept
  {
    _context.Shaders().Unload(_uiData.Shader);
    _context.Buffers().Destroy(_uiData.Vbo);
    _context.Buffers().Destroy(_uiData.Ebo);
    _context.Buffers().Destroy(_uiData.InstanceDataBuffer);
    glDeleteVertexArrays(1, &_uiData.Vao);
  }

  void Renderer::BeginFrame()
  {
    _uiData.InstanceData.clear();
  }

  void Renderer::EndFrame()
  {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    {
      auto &shader = static_cast<ShaderRegistry &>(_context.Shaders()).Get(_uiData.Shader);
      shader.Bind();

      auto &buffer = static_cast<BufferRegistry &>(_context.Buffers()).Get(_uiData.InstanceDataBuffer);
      buffer.Update(_uiData.InstanceData);

      glBindVertexArray(_uiData.Vao);
      uint32 instanceCount = static_cast<GLsizei>(_uiData.InstanceData.size());

      glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr, instanceCount);
    }
    glDisable(GL_BLEND);
  }

  void Renderer::Submit(const CommandList &commandList)
  {
    CommandListReader reader(commandList);
    while (reader.HasMore())
    {
      CommandHeader header = reader.ReadHeader();
      switch (header.Type)
      {
        case KRYS_CMD_TYPE("RectCommand"):
        {
          const auto &cmd = reader.ReadCommand<RectCommand>();
          _uiData.InstanceData.push_back({
            .BackgroundColour = cmd.BackgroundColour,
            .BorderColour = cmd.BorderColour,
            .PositionAndSize = {cmd.Position.x, cmd.Position.y, cmd.Size.x, cmd.Size.y},
            .BorderThicknessRadius = {cmd.BorderThickness, cmd.BorderRadius},
          });
          break;
        }
        default: KRYS_WARN("Unknown command type submitted to OpenGL renderer: {}", header.Type); break;
      }
    }
  }
}