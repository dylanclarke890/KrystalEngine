#include "Krystal.Gfx.OpenGL/Renderer.hpp"
#include "Krystal.Gfx.OpenGL/Debug.hpp"
#include "Krystal.Gfx/Commands/CommandListReader.hpp"
#include "Krystal.Gfx/Commands/Commands.hpp"
#include "Krystal.Gfx/Vertex.hpp"
#include "Krystal.Lib/Expected.hpp"
#include "Krystal.Lib/String/UTF8.hpp"
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
      : _context(static_cast<Context &>(context)), _dpi(Platform::GetDPIForWindow())
  {
  }

  void Renderer::Startup()
  {
    auto &buffers = static_cast<BufferRegistry &>(_context.Buffers());
    auto &meshes = static_cast<MeshRegistry &>(_context.Meshes());
    auto &renderTargets = static_cast<RenderTargetRegistry &>(_context.RenderTargets());
    auto &shaders = static_cast<ShaderRegistry &>(_context.Shaders());

    _quadShader = shaders.Load(IO::Path("ui.vert"), IO::Path("ui.frag"));
    Debug::SetName(shaders.Get(_quadShader), "UIQuadShader");

    _singleTextureShader = shaders.Load(IO::Path("single-texture.vert"), IO::Path("single-texture.frag"));
    Debug::SetName(shaders.Get(_singleTextureShader), "SingleTextureShader");

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
      Debug::SetName(quadInstanceBuffer, "QuadInstanceData");
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
    Debug::SetName(buffers.Get(_glyphBuffer), "GlyphVertexData");
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
    auto &textures = static_cast<TextureRegistry &>(_context.Textures());
    auto &renderTargets = static_cast<RenderTargetRegistry &>(_context.RenderTargets());
    while (reader.HasMore())
    {
      CommandHeader header = reader.ReadHeader();
      switch (header.Type)
      {
        case Commands::SetScissor::Type:
        {
          const auto &cmd = reader.ReadCommand<Commands::SetScissor>();
          auto &rt = renderTargets.Get(_currentRenderTarget);

          GLint scissorX = static_cast<GLint>(cmd.Position.x);
          GLint scissorY = static_cast<GLint>(rt.Height() - (cmd.Position.y + cmd.Size.y));
          GLsizei scissorWidth = static_cast<GLsizei>(cmd.Size.x);
          GLsizei scissorHeight = static_cast<GLsizei>(cmd.Size.y);

          glEnable(GL_SCISSOR_TEST);
          glScissor(scissorX, scissorY, scissorWidth, scissorHeight);
          break;
        }
        case Commands::ClearScissor::Type:
        {
          const auto &cmd = reader.ReadCommand<Commands::ClearScissor>();
          KRYS_UNUSED(cmd);

          glDisable(GL_SCISSOR_TEST);
          break;
        }
        case Commands::SetViewport::Type:
        {
          const auto &cmd = reader.ReadCommand<Commands::SetViewport>();
          auto &rt = renderTargets.Get(_currentRenderTarget);
          GLint viewportX = static_cast<GLint>(cmd.Position.x);
          GLint viewportY = static_cast<GLint>(rt.Height() - (cmd.Position.y + cmd.Size.y));
          GLsizei viewportWidth = static_cast<GLsizei>(cmd.Size.x);
          GLsizei viewportHeight = static_cast<GLsizei>(cmd.Size.y);
          glViewport(viewportX, viewportY, viewportWidth, viewportHeight);
          break;
        }
        case Commands::BindRenderTarget::Type:
        {
          const auto &cmd = reader.ReadCommand<Commands::BindRenderTarget>();
          assert(cmd.RenderTarget.IsValid() && "Invalid render target handle in BindRenderTarget.");

          if (cmd.RenderTarget == _currentRenderTarget)
          {
            break;
          }

          auto &rt = renderTargets.Get(cmd.RenderTarget);
          renderTargets.Bind(cmd.RenderTarget);

          glViewport(0, 0, rt.Width(), rt.Height());
          glDisable(GL_SCISSOR_TEST);

          _currentRenderTarget = cmd.RenderTarget;
          shaders.Get(_quadShader).SetUniform("u_Projection", rt.GetProjectionMatrix());
          shaders.Get(_singleTextureShader).SetUniform("u_Projection", rt.GetProjectionMatrix());
          break;
        }
        case Commands::DrawText::Type:
        {
          const auto &cmd = reader.ReadCommand<Commands::DrawText>();
          auto &rt = renderTargets.Get(_currentRenderTarget);

          FontHandle fontHandle =
            fonts.Get({.Family = cmd.FontFamily, .Type = FontType::Bitmap, .Size = cmd.FontSize});
          Font &font = fonts.Get(fontHandle);
          const auto &fontMetrics = fonts.GetMetrics(fontHandle);

          float scale = 1.f;
          if (font.Type() != FontType::Bitmap)
          {
            scale = fonts.PtSizeToPixels(cmd.FontSize) / font.SDFParams().EMSizeInPixels;
          }

          DrawText(_context.Strings().Get(cmd.Text), fontHandle, cmd.FontSize, cmd.Position, cmd.Colour);
          break;
        }
        case Commands::DrawShape2D::Type:
        {
          const auto &cmd = reader.ReadCommand<Commands::DrawShape2D>();

          auto &mesh = static_cast<MeshRegistry &>(_context.Meshes()).Get(cmd.Mesh);
          auto &rt = renderTargets.Get(_currentRenderTarget);
          Maths::Mat4 projection = rt.GetProjectionMatrix();

          ShaderHandle shaderHandle;
          if (cmd.Texture.IsValid())
          {
            shaderHandle = shaders.GetBuiltin(BuiltinShader::Shape2D_Texture);
            textures.Bind(cmd.Texture, 0);
            shaders.Get(shaderHandle).SetUniform("u_Texture", 0);
          }
          else
          {
            shaderHandle = shaders.GetBuiltin(BuiltinShader::Shape2D_Colour);
          }

          auto &shader = shaders.Get(shaderHandle);
          shader.Bind();
          shader.SetUniform("u_Transform", projection * cmd.Transform);

          const Maths::Vec2 translation = {cmd.Translation.x, cmd.Translation.y};
          shader.SetUniform("u_Translate", translation);
          mesh.Bind();
          mesh.DrawInstanced(static_cast<GLsizei>(cmd.InstanceCount));
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

  void Renderer::DrawTexturedQuad(GLuint texture, const Maths::Vec2 &position, const Maths::Vec2 &size,
                                  float opacity)
  {
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    auto &shader = static_cast<ShaderRegistry &>(_context.Shaders()).Get(_singleTextureShader);
    shader.Bind();
    shader.SetUniform("u_Texture", 0);
    shader.SetUniform("u_Opacity", opacity);

    glBindTextureUnit(0, texture);

    // Setup instance data for just one quad
    Array<QuadInstanceData, 1> instance = {QuadInstanceData {
      .PositionAndSize = {position.x, position.y, size.x, size.y},
      .BorderWidths = Maths::Vec4 {0.f},
      .BackgroundColour = Gfx::Colours::Transparent,
      .BorderColourLeft = Gfx::Colours::Transparent,
      .BorderColourRight = Gfx::Colours::Transparent,
      .BorderColourTop = Gfx::Colours::Transparent,
      .BorderColourBottom = Gfx::Colours::Transparent,
    }};

    auto &buffer = static_cast<BufferRegistry &>(_context.Buffers()).Get(_quadInstanceData.Buffer);
    buffer.Update(instance);

    auto &mesh = static_cast<MeshRegistry &>(_context.Meshes()).Get(_quadMesh);
    mesh.Bind();
    mesh.DrawInstanced(1);

    glDisable(GL_BLEND);
  }

  void Renderer::DrawText(const utf8_string &text, FontHandle fontHandle, float ptSize,
                          const Maths::Vec2 &position, const ColourbPremultiplied &colour) noexcept
  {
    Font &font = static_cast<FontRegistry &>(_context.Fonts()).Get(fontHandle);

    BuiltinShader builtin = [&]()
    {
      switch (font.Type())
      {
        case FontType::Bitmap: return BuiltinShader::Font_Bitmap;
        case FontType::SDF:    return BuiltinShader::Font_SDF;
        case FontType::MSDF:   return BuiltinShader::Font_MSDF;
        case FontType::MTSDF:  return BuiltinShader::Font_MTSDF;
      }
      std::unreachable();
    }();

    auto &shaders = static_cast<ShaderRegistry &>(_context.Shaders());

    ShaderHandle handle = shaders.GetBuiltin(builtin);
    DrawText(font, shaders.Get(handle), text, colour, position, ptSize);
  }

  void Renderer::DrawTextOutlined(const utf8_string &text, FontHandle fontHandle, float ptSize,
                                  const Maths::Vec2 &position, const ColourbPremultiplied &textColour,
                                  const ColourbPremultiplied &outlineColour, float outlineWidth) noexcept
  {
    Font &font = static_cast<FontRegistry &>(_context.Fonts()).Get(fontHandle);
    assert(font.Type() != FontType::Bitmap && "Outlined text is not supported for bitmap fonts.");

    BuiltinShader builtin = [&]()
    {
      switch (font.Type())
      {
        case FontType::SDF:   return BuiltinShader::Font_SDF_Outline;
        case FontType::MSDF:  return BuiltinShader::Font_MSDF_Outline;
        case FontType::MTSDF: return BuiltinShader::Font_MTSDF_Outline;
      }
      std::unreachable();
    }();

    auto &shaders = static_cast<ShaderRegistry &>(_context.Shaders());
    ShaderHandle handle = shaders.GetBuiltin(builtin);
    Shader &shader = shaders.Get(handle);

    shader.SetUniform("u_OutlineColor", outlineColour.ToVec3());
    shader.SetUniform("u_OutlineWidthAbsolute", outlineWidth / 3.f);
    shader.SetUniform("u_OutlineWidthRelative", outlineWidth / 20.f);
    shader.SetUniform("u_Threshold", 0.5f);

    DrawText(font, shader, text, textColour, position, ptSize);
  }

  void Renderer::DrawText(Font &font, Shader &shader, const utf8_string &text,
                          const ColourbPremultiplied &textColour, const Maths::Vec2 &position, float ptSize)
  {
    glBindVertexArray(_textVao);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    Maths::Vec2 pos = position;
    Buffer &buffer = static_cast<BufferRegistry &>(_context.Buffers()).Get(_glyphBuffer);
    auto &fonts = static_cast<FontRegistry &>(_context.Fonts());

    static_cast<TextureRegistry &>(_context.Textures()).Bind(font.AtlasTexture(), 0u);

    shader.Bind();
    shader.SetUniform("u_TextColor", textColour.ToVec3());

    float scale = 1.f;
    if (font.Type() != FontType::Bitmap)
    {
      scale = fonts.PtSizeToPixels(ptSize) / font.SDFParams().EMSizeInPixels;
      auto unitRange = Maths::Vec2(font.SDFParams().PixelRange) / Maths::Vec2(font.AtlasSize());
      shader.SetUniform("u_UnitRange", unitRange);
    }

    List<Codepoint> codepoints = UTF8::Decode(text);
    auto count = codepoints.size();
    while (count > 0)
    {
      auto batchSize = Maths::Min(count, static_cast<size_t>(GlyphVertex::BatchSize));
      Span<const Codepoint> batch(codepoints.data() + (codepoints.size() - count), batchSize);
      count -= batchSize;

      _glyphVertices.clear();
      const auto &characters = font.Characters();
      for (const Codepoint &c : batch)
      {
        // Check for newline character
        if (c.Value == '\n')
        {
          pos.x = position.x;
          pos.y += font.Metrics().Height * scale;
          continue;
        }

        const auto &glyph = characters.find(c);
        if (glyph == characters.end())
        {
          // TODO: this check is better than before but we should default to using a missing glyph character
          KRYS_WARN("Font '{}' does not contain glyph for character '{}'", font.Family().Id, c.Value);
          continue;
        }

        const Character &ch = glyph->second;
        float posX = pos.x + ch.Bearing.x * scale;
        float posY = pos.y + (ch.Size.y - ch.Bearing.y) * scale;
        float w = ch.Size.x * scale;
        float h = ch.Size.y * scale;

        _glyphVertices.push_back({{posX, posY + h}, {ch.UVMin.x, ch.UVMin.y}});
        _glyphVertices.push_back({{posX, posY}, {ch.UVMin.x, ch.UVMax.y}});
        _glyphVertices.push_back({{posX + w, posY}, {ch.UVMax.x, ch.UVMax.y}});
        _glyphVertices.push_back({{posX, posY + h}, {ch.UVMin.x, ch.UVMin.y}});
        _glyphVertices.push_back({{posX + w, posY}, {ch.UVMax.x, ch.UVMax.y}});
        _glyphVertices.push_back({{posX + w, posY + h}, {ch.UVMax.x, ch.UVMin.y}});

        if (font.Type() != FontType::Bitmap && ptSize < 24.f)
        {
          pos.x += std::floor(ch.Advance + 0.5f) * scale;
        }
        else
        {
          pos.x += ch.Advance * scale;
        }
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