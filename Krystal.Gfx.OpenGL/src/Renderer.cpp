#include "Krystal.Gfx.OpenGL/Renderer.hpp"
#include "Krystal.Gfx.OpenGL/Debug.hpp"
#include "Krystal.Gfx/Commands/CommandListReader.hpp"
#include "Krystal.Gfx/Commands/Commands.hpp"
#include "Krystal.Gfx/Vertex.hpp"
#include "Krystal.Lib/Expected.hpp"
#include "Krystal.Lib/String/UTF8.hpp"
#include "Krystal.Maths/Clamp.hpp"
#include "Krystal.Maths/Transform.hpp"

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
    auto &buffers = static_cast<BufferRegistry &>(_context.Buffers());
    auto &meshes = static_cast<MeshRegistry &>(_context.Meshes());
    auto &renderTargets = static_cast<RenderTargetRegistry &>(_context.RenderTargets());
    auto &shaders = static_cast<ShaderRegistry &>(_context.Shaders());

    _state.CurrentRenderTarget = _context.RenderTargets().GetScreenRenderTarget();
    auto &rt = renderTargets.Get(_state.CurrentRenderTarget);

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
    auto &meshes = static_cast<MeshRegistry &>(_context.Meshes());
    auto &shaders = static_cast<ShaderRegistry &>(_context.Shaders());
    auto &textures = static_cast<TextureRegistry &>(_context.Textures());
    auto &renderTargets = static_cast<RenderTargetRegistry &>(_context.RenderTargets());

    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    while (reader.HasMore())
    {
      CommandHeader header = reader.ReadHeader();
      switch (header.Type)
      {
        case Commands::SetScissor::Type:
        {
          const auto &cmd = reader.ReadCommand<Commands::SetScissor>();
          auto &rt = renderTargets.Get(_state.CurrentRenderTarget);

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
          auto &rt = renderTargets.Get(_state.CurrentRenderTarget);
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
          if (cmd.RenderTarget == _state.CurrentRenderTarget)
          {
            break;
          }

          renderTargets.Bind(cmd.RenderTarget);
          auto &rt = renderTargets.Get(cmd.RenderTarget);
          glViewport(0, 0, rt.Width(), rt.Height());
          glDisable(GL_SCISSOR_TEST);

          _state.CurrentRenderTarget = cmd.RenderTarget;
          break;
        }
        case Commands::DrawShape2D::Type:
        {
          const auto &cmd = reader.ReadCommand<Commands::DrawShape2D>();

          ShaderHandle shaderHandle;
          if (cmd.Texture.IsValid())
          {
            shaderHandle = shaders.GetBuiltin(BuiltinShader::Shape2D_Texture);
            textures.Bind(cmd.Texture, 0u);
            shaders.Get(shaderHandle).SetUniform("u_Texture", 0);
          }
          else
          {
            shaderHandle = shaders.GetBuiltin(BuiltinShader::Shape2D_Colour);
          }

          auto &rt = renderTargets.Get(_state.CurrentRenderTarget);
          auto &shader = shaders.Get(shaderHandle);
          auto &mesh = meshes.Get(cmd.Mesh);

          shader.Bind();
          shader.SetUniform("u_Transform", rt.GetProjectionMatrix() * cmd.Transform);

          mesh.Bind();
          mesh.Draw(static_cast<GLsizei>(cmd.InstanceCount));
          break;
        }
        case Commands::ComposeRenderTargets::Type:
        {
          using namespace Maths;

          const auto &cmd = reader.ReadCommand<Commands::ComposeRenderTargets>();

          auto &destRenderTarget = renderTargets.Get(cmd.Destination);
          renderTargets.Bind(cmd.Destination);
          glViewport(0, 0, destRenderTarget.Width(), destRenderTarget.Height());

          ShaderHandle shaderHandle = shaders.GetBuiltin(BuiltinShader::Shape2D_Texture);
          auto &shader = shaders.Get(shaderHandle);
          shader.Bind();

          auto &srcRenderTarget = renderTargets.Get(cmd.Source);
          glBindTextureUnit(0u, srcRenderTarget.GetColourAttachment(0u).Texture);
          shader.SetUniform("u_Texture", 0);

          Mat4 transform = Scale(Vec3 {(float)srcRenderTarget.Width(), (float)srcRenderTarget.Height(), 1.f});
          Mat4 projection = destRenderTarget.GetProjectionMatrix();
          shader.SetUniform("u_Transform", projection * transform);

          glBlendFunc(GL_CONSTANT_COLOR, GL_ZERO);
          glBlendColor(cmd.Opacity, cmd.Opacity, cmd.Opacity, cmd.Opacity);

          auto &mesh = meshes.Get(meshes.GetFullScreenQuad());
          mesh.Bind();
          mesh.Draw();

          if (_state.CurrentRenderTarget != cmd.Destination)
          {
            // Restore previous render target
            renderTargets.Bind(_state.CurrentRenderTarget);
            glViewport(0, 0, renderTargets.Get(_state.CurrentRenderTarget).Width(),
                       renderTargets.Get(_state.CurrentRenderTarget).Height());
          }

          glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
          break;
        }
        case Commands::DrawText::Type:
        {
          const auto &cmd = reader.ReadCommand<Commands::DrawText>();

          FontHandle fontHandle = fonts.Get({cmd.FontFamily, FontType::Bitmap, cmd.FontSize});
          Font &font = fonts.Get(fontHandle);
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

          Shader &shader = shaders.Get(shaders.GetBuiltin(builtin));
          DrawText(font, shader, _context.Strings().Get(cmd.Text), cmd.Colour, cmd.Position, cmd.FontSize);
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

    glDisable(GL_BLEND);
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
    auto &fonts = static_cast<FontRegistry &>(_context.Fonts());
    auto &textures = static_cast<TextureRegistry &>(_context.Textures());
    auto &renderTargets = static_cast<RenderTargetRegistry &>(_context.RenderTargets());
    auto &buffers = static_cast<BufferRegistry &>(_context.Buffers());

    float scale = 1.f;
    if (font.Type() != FontType::Bitmap)
    {
      scale = fonts.PtSizeToPixels(ptSize) / font.SDFParams().EMSizeInPixels;
      auto unitRange = Maths::Vec2(font.SDFParams().PixelRange) / Maths::Vec2(font.AtlasSize());
      shader.SetUniform("u_UnitRange", unitRange);
    }

    glBindVertexArray(_textVao);

    shader.Bind();
    shader.SetUniform("u_TextColor", textColour.ToVec3());
    shader.SetUniform("u_Transform", renderTargets.Get(_state.CurrentRenderTarget).GetProjectionMatrix());
    textures.Bind(font.AtlasTexture(), 0u);

    Buffer &buffer = buffers.Get(_glyphBuffer);
    Maths::Vec2 cursor = position + Maths::Vec2 {0.f, font.Metrics().Ascender * scale};
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
          cursor.x = position.x;
          cursor.y += font.Metrics().Height * scale;
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
        float posX = cursor.x + (ch.Bearing.x * scale);
        float posY = cursor.y - (ch.Bearing.y * scale);
        float w = ch.Size.x * scale;
        float h = ch.Size.y * scale;

        _glyphVertices.push_back({{posX, posY + h}, {ch.UVMin.x, ch.UVMax.y}});
        _glyphVertices.push_back({{posX, posY}, {ch.UVMin.x, ch.UVMin.y}});
        _glyphVertices.push_back({{posX + w, posY}, {ch.UVMax.x, ch.UVMin.y}});
        _glyphVertices.push_back({{posX, posY + h}, {ch.UVMin.x, ch.UVMax.y}});
        _glyphVertices.push_back({{posX + w, posY}, {ch.UVMax.x, ch.UVMin.y}});
        _glyphVertices.push_back({{posX + w, posY + h}, {ch.UVMax.x, ch.UVMax.y}});

        if (font.Type() != FontType::Bitmap && ptSize < 24.f)
        {
          cursor.x += std::floor(ch.Advance + 0.5f) * scale;
        }
        else
        {
          cursor.x += ch.Advance * scale;
        }
      }

      buffer.Update(_glyphVertices);
      glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(_glyphVertices.size()));
    }
  }
}