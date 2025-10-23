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
    _quadShader = _context.Shaders().Load(IO::Path("ui.vert"), IO::Path("ui.frag"));

    auto &buffers = static_cast<BufferRegistry &>(_context.Buffers());
    auto &meshes = static_cast<MeshRegistry &>(_context.Meshes());

    _quadMesh = meshes.CreateQuad();
    auto &quadMesh = meshes.Get(_quadMesh);

    const size_t batchSize = 1'000;
    _quadInstanceData.Buffer = buffers.Create({
      .Type = BufferType::Vertex,
      .Usage = BufferUsage::Dynamic,
      .Size = batchSize * sizeof(QuadInstanceData),
    });
    _quadInstanceData.Data.reserve(batchSize);

    auto &quadInstanceBuffer = buffers.Get(_quadInstanceData.Buffer);
    quadMesh.ApplyInstanceDataLayout(quadInstanceBuffer, QuadInstanceData::Layout());
    glObjectLabel(GL_BUFFER, quadInstanceBuffer.GetHandle(), -1, "QuadInstanceData");
  }

  void Renderer::Shutdown() noexcept
  {
    _context.Shaders().Unload(_quadShader);
    _context.Buffers().Destroy(_quadInstanceData.Buffer);
    _context.Meshes().Destroy(_quadMesh);
  }

  void Renderer::BeginFrame()
  {
    _quadInstanceData.Data.clear();
  }

  void Renderer::EndFrame()
  {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    {
      auto &shader = static_cast<ShaderRegistry &>(_context.Shaders()).Get(_quadShader);
      shader.Bind();

      auto &buffer = static_cast<BufferRegistry &>(_context.Buffers()).Get(_quadInstanceData.Buffer);
      buffer.Update(_quadInstanceData.Data);

      auto &mesh = static_cast<MeshRegistry &>(_context.Meshes()).Get(_quadMesh);
      mesh.Bind();
      mesh.DrawInstanced(static_cast<GLsizei>(_quadInstanceData.Data.size()));
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
          float flippedY = _context.Height() - (cmd.Position.y + cmd.Size.y);
          _quadInstanceData.Data.push_back({
            .BackgroundColour = cmd.BackgroundColour,
            .BorderColour = cmd.BorderColour,
            .PositionAndSize = {cmd.Position.x, flippedY, cmd.Size.x, cmd.Size.y},
            .BorderThicknessRadius = {cmd.BorderThickness, cmd.BorderRadius},
          });
          break;
        }
        default:
        {
          KRYS_WARN("Unknown command type submitted to OpenGL renderer, skipping: {}", header.Type);
          reader.SkipBytes(header.SizeInBytes);
          break;
        }
      }
    }
  }
}