#include "Krystal.Gfx.OpenGL/Renderer.hpp"
#include "Krystal.Gfx/Commands/CommandListReader.hpp"
#include "Krystal.Gfx/Commands/Commands.hpp"
#include "Krystal.Gfx/Vertex.hpp"
#include "Krystal.Lib/Expected.hpp"
#include "Krystal.Maths/Clamp.hpp"
#include "Krystal.Platform/Platform.hpp"

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
  Renderer::Renderer(IContext &context) noexcept
      : _context(static_cast<Context &>(context)),
        _dpi(Platform::GetDPIForWindow(Platform::GetActiveWindow()))
  {
  }

  void Renderer::Startup()
  {
    auto &buffers = static_cast<BufferRegistry &>(_context.Buffers());
    auto &meshes = static_cast<MeshRegistry &>(_context.Meshes());
    auto &renderTargets = static_cast<RenderTargetRegistry &>(_context.RenderTargets());
    auto &shaders = static_cast<ShaderRegistry &>(_context.Shaders());

    _quadShader = shaders.Load(IO::Path("ui.vert"), IO::Path("ui.frag"));
    _singleTextureShader = shaders.Load(IO::Path("single-texture.vert"), IO::Path("single-texture.frag"));

    {
      _quadMesh = meshes.CreateQuad();
      auto &quadMesh = meshes.Get(_quadMesh);

      _quadInstanceData.Buffer = buffers.Create({
        .Type = BufferType::Vertex,
        .Usage = BufferUsage::Dynamic,
        .Size = _quadInstanceData.BufferSize,
      });
      _quadInstanceData.Data.reserve(_quadInstanceData.BatchSize);

      auto &quadInstanceBuffer = buffers.Get(_quadInstanceData.Buffer);
      quadMesh.ApplyInstanceDataLayout(quadInstanceBuffer, QuadInstanceData::Layout());
      glObjectLabel(GL_BUFFER, quadInstanceBuffer.GetHandle(), -1, "QuadInstanceData");
    }

    _currentRenderTarget = _context.RenderTargets().GetScreenRenderTarget();
    auto &rt = renderTargets.Get(_currentRenderTarget);

    shaders.Get(_quadShader).SetUniform("u_Projection", rt.GetProjectionMatrix());
    shaders.Get(_singleTextureShader).SetUniform("u_Projection", rt.GetProjectionMatrix());

    uint32 maxGlyphVertices = GlyphVertex::VerticesPerGlyph * GlyphVertex::BatchSize;
    _glyphBuffer = buffers.Create({
      .Type = BufferType::Vertex,
      .Usage = BufferUsage::Dynamic,
      .Size = static_cast<uint32>(sizeof(GlyphVertex)) * maxGlyphVertices,
      .InitialData = {},
    });
    _glyphVertices.reserve(maxGlyphVertices);

    glCreateVertexArrays(1, &_textVao);
    glBindVertexArray(_textVao);
    buffers.Get(_glyphBuffer).Bind();
    Utils::ApplyVertexBufferLayout(GlyphVertex::Layout());
    glObjectLabel(GL_BUFFER, buffers.Get(_glyphBuffer).GetHandle(), -1, "GlyphVertexData");
  }

  void Renderer::Shutdown() noexcept
  {
    _context.Shaders().Unload(_quadShader);
    _context.Buffers().Destroy(_quadInstanceData.Buffer);
    _context.Meshes().Destroy(_quadMesh);
  }

  void Renderer::BeginFrame()
  {
  }

  void Renderer::EndFrame()
  {
  }

  void Renderer::Submit(const CommandList &commandList)
  {
    CommandListReader reader(commandList);

    auto &fonts = static_cast<FontRegistry &>(_context.Fonts());
    auto &shaders = static_cast<ShaderRegistry &>(_context.Shaders());
    auto &renderTargets = static_cast<RenderTargetRegistry &>(_context.RenderTargets());
    while (reader.HasMore())
    {
      CommandHeader header = reader.ReadHeader();
      switch (header.Type)
      {
        case Commands::BindRenderTarget::Type:
        {
          const auto &cmd = reader.ReadCommand<Commands::BindRenderTarget>();
          assert(cmd.RenderTarget.IsValid() && "Invalid render target handle in BindRenderTarget.");
          if (cmd.RenderTarget == _currentRenderTarget)
          {
            break;
          }
          FlushQuadInstances();

          auto &rt = renderTargets.Get(cmd.RenderTarget);
          renderTargets.Bind(cmd.RenderTarget);
          glViewport(0, 0, rt.Width(), rt.Height());
          _currentRenderTarget = cmd.RenderTarget;
          shaders.Get(_quadShader).SetUniform("u_Projection", rt.GetProjectionMatrix());
          shaders.Get(_singleTextureShader).SetUniform("u_Projection", rt.GetProjectionMatrix());
          break;
        }
        case Commands::DrawRect::Type:
        {
          const auto &cmd = reader.ReadCommand<Commands::DrawRect>();
          auto &rt = renderTargets.Get(_currentRenderTarget);

          float posY = static_cast<float>(rt.Height()) - (cmd.Position.y + cmd.Size.y);
          _quadInstanceData.Data.push_back({
            .BackgroundColour = cmd.BackgroundColour,
            .BorderColour = cmd.BorderColour,
            .PositionAndSize = {cmd.Position.x, posY, cmd.Size.x, cmd.Size.y},
            .BorderThicknessRadius = {cmd.BorderThickness, cmd.BorderRadius},
          });

          if (_quadInstanceData.Data.size() >= QuadInstanceData::BatchSize)
          {
            FlushQuadInstances();
          }
          break;
        }
        case Commands::DrawRenderTargetColourAttachment::Type:
        {
          FlushQuadInstances();

          const auto &cmd = reader.ReadCommand<Commands::DrawRenderTargetColourAttachment>();
          auto &sourceRT = renderTargets.Get(cmd.Source);
          const auto &attachment = sourceRT.GetColourAttachment(cmd.ColourAttachmentIndex);
          const auto &imageView =
            static_cast<ImageViewRegistry &>(_context.ImageViews()).Get(attachment.ImageView);
          auto &dstRT = renderTargets.Get(_currentRenderTarget);

          float posY = static_cast<float>(dstRT.Height()) - (cmd.Position.y + cmd.Size.y);
          DrawTexturedQuad(imageView.Id(), {cmd.Position.x, posY}, cmd.Size, cmd.Opacity);
          break;
        }
        case Commands::DrawText::Type:
        {
          FlushQuadInstances();
          const auto &cmd = reader.ReadCommand<Commands::DrawText>();
          auto &rt = renderTargets.Get(_currentRenderTarget);

          {
            FontHandle font =
              fonts.Get({.Family = cmd.FontFamily, .Type = FontType::Bitmap, .Size = cmd.FontSize});
            const auto &metrics = fonts.GetMetrics(font);
            float posY = static_cast<float>(rt.Height()) - (cmd.Position.y + metrics.Ascender);
            DrawText(_context.Strings().Get(cmd.Text), font, cmd.FontSize, {cmd.Position.x, posY},
                     Colours::Red);
          }

          {
            FontHandle font =
              fonts.Get({.Family = cmd.FontFamily, .Type = FontType::SDF, .Size = cmd.FontSize});
            float scale = ((_dpi / 72.f) * cmd.FontSize) / fonts.Get(font).SDFParams().EMSizeInPixels;

            const auto &metrics = fonts.GetMetrics(font);
            float posY = static_cast<float>(rt.Height()) - (cmd.Position.y + (metrics.Ascender * scale));
            DrawText(_context.Strings().Get(cmd.Text), font, cmd.FontSize, {cmd.Position.x, posY},
                     Colours::Blue);
          }

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

    FlushQuadInstances();
  }

  void Renderer::FlushQuadInstances()
  {
    if (_quadInstanceData.Data.empty())
    {
      return;
    }

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
    _quadInstanceData.Data.clear();
  }

  void Renderer::DrawTexturedQuad(GLuint texture, const Maths::Vec2 &position, const Maths::Vec2 &size,
                                  float opacity)
  {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    auto &shader = static_cast<ShaderRegistry &>(_context.Shaders()).Get(_singleTextureShader);
    shader.Bind();
    shader.SetUniform("u_Texture", 0);
    shader.SetUniform("u_Opacity", opacity);

    glBindTextureUnit(0, texture);

    // Setup instance data for just one quad
    Array<QuadInstanceData, 1> instance;
    instance[0].BackgroundColour = Gfx::Colours::Black;
    instance[0].BorderColour = Gfx::Colours::Transparent;
    instance[0].PositionAndSize = {position.x, position.y, size.x, size.y};
    instance[0].BorderThicknessRadius = {0, 0};

    auto &buffer = static_cast<BufferRegistry &>(_context.Buffers()).Get(_quadInstanceData.Buffer);
    buffer.Update(instance);

    auto &mesh = static_cast<MeshRegistry &>(_context.Meshes()).Get(_quadMesh);
    mesh.Bind();
    mesh.DrawInstanced(1);

    glDisable(GL_BLEND);
  }

  void Renderer::DrawText(const string &text, FontHandle fontHandle, float ptSize,
                          const Maths::Vec2 &position, const Colour &colour) noexcept
  {
    Font &font = static_cast<FontRegistry &>(_context.Fonts()).Get(fontHandle);
    TextShaderDesc shaderDesc = {.FontType = font.Type()};
    Shader &shader = static_cast<ShaderRegistry &>(_context.Shaders()).GetOrAdd(shaderDesc);

    DrawText(font, shader, text, colour, position, ptSize);
  }

  void Renderer::DrawTextOutlined(const string &text, FontHandle fontHandle, float ptSize,
                                  const Maths::Vec2 &position, const Colour &textColour,
                                  const Colour &outlineColour, float outlineWidth) noexcept
  {
    Font &font = static_cast<FontRegistry &>(_context.Fonts()).Get(fontHandle);
    assert(font.Type() != FontType::Bitmap && "Outlined text is not supported for bitmap fonts.");
    TextShaderDesc shaderDesc = {.FontType = font.Type(), .EnableOutline = true};
    Shader &shader = static_cast<ShaderRegistry &>(_context.Shaders()).GetOrAdd(shaderDesc);

    shader.SetUniform("u_OutlineColor", outlineColour.ToVec3());
    shader.SetUniform("u_OutlineWidthAbsolute", outlineWidth / 3.f);
    shader.SetUniform("u_OutlineWidthRelative", outlineWidth / 20.f);
    shader.SetUniform("u_Threshold", 0.5f);

    DrawText(font, shader, text, textColour, position, ptSize);
  }

  void Renderer::DrawText(Font &font, Shader &shader, const string &text, const Colour &textColour,
                          const Maths::Vec2 &position, float ptSize)
  {
    glBindVertexArray(_textVao);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    Maths::Vec2 pos = position;
    Buffer &buffer = static_cast<BufferRegistry &>(_context.Buffers()).Get(_glyphBuffer);

    Texture &texture = static_cast<TextureRegistry &>(_context.Textures()).Get(font.AtlasTexture());
    ImageView &imageView = static_cast<ImageViewRegistry &>(_context.ImageViews()).Get(texture.ImageView());
    imageView.Bind(0);

    shader.Bind();
    shader.SetUniform("u_TextColor", textColour.ToVec3());

    float scale = 1.f;
    if (font.Type() != FontType::Bitmap)
    {
      scale = ((_dpi / 72.f) * ptSize) / font.SDFParams().EMSizeInPixels;
      auto unitRange = Maths::Vec2(font.SDFParams().PixelRange) / Maths::Vec2(font.AtlasSize());
      shader.SetUniform("u_UnitRange", unitRange);
    }

    auto count = text.size();
    while (count > 0)
    {
      auto batchSize = Maths::Min(count, static_cast<size_t>(GlyphVertex::BatchSize));
      Span<const char> batch(text.data() + (text.size() - count), batchSize);
      count -= batchSize;

      _glyphVertices.clear();
      const auto &characters = font.Characters();
      for (const char c : batch)
      {
        // TODO: better checks for whether the character exists before accessing it
        const Character &ch = characters.at(Codepoint(c));
        float posX = pos.x + ch.Bearing.x * scale;
        float posY = pos.y - (ch.Size.y - ch.Bearing.y) * scale;
        float w = ch.Size.x * scale;
        float h = ch.Size.y * scale;

        _glyphVertices.push_back({{posX, posY + h}, {ch.UVMin.x, ch.UVMin.y}});
        _glyphVertices.push_back({{posX, posY}, {ch.UVMin.x, ch.UVMax.y}});
        _glyphVertices.push_back({{posX + w, posY}, {ch.UVMax.x, ch.UVMax.y}});
        _glyphVertices.push_back({{posX, posY + h}, {ch.UVMin.x, ch.UVMin.y}});
        _glyphVertices.push_back({{posX + w, posY}, {ch.UVMax.x, ch.UVMax.y}});
        _glyphVertices.push_back({{posX + w, posY + h}, {ch.UVMax.x, ch.UVMin.y}});

        pos.x += ch.Advance * scale;
      }

      buffer.Update(_glyphVertices);
      glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(_glyphVertices.size()));
    }

    glDisable(GL_BLEND);
  }

  void Renderer::DPIChanged(int dpi) noexcept
  {
    _dpi = dpi;
  }
}