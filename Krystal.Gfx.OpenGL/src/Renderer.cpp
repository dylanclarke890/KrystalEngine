#include "Krystal.Gfx.OpenGL/Renderer.hpp"
#include "Krystal.Gfx.OpenGL/Debug.hpp"
#include "Krystal.Gfx.OpenGL/Mappers/Enums/BufferBitFlags.hpp"
#include "Krystal.Gfx.OpenGL/Mappers/Enums/FilterMode.hpp"
#include "Krystal.Gfx/Commands.hpp"
#include "Krystal.Gfx/Enums/BufferBitFlags.hpp"
#include "Krystal.Gfx/Vertex.hpp"
#include "Krystal.Lib/Commands/CommandListReader.hpp"
#include "Krystal.Lib/Types/Expected.hpp"
#include "Krystal.Maths/Clamp.hpp"
#include "Krystal.Maths/Transform.hpp"
#include "Krystal.Text/Decode/Decode.hpp"
#include "Krystal.Text/Encodings/UTF8.hpp"
#include "Krystal.Text/UnicodeCodePoint.hpp"

namespace Krys::Gfx
{
  Expected<UniquePtr<IRenderer>> CreateRenderer(IContext &context) noexcept
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

    glDisable(GL_DEPTH_TEST);
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
          KRYS_MAYBE_UNUSED const auto &cmd = reader.ReadCommand<Commands::ClearScissor>();

          glDisable(GL_SCISSOR_TEST);
          break;
        }
        case Commands::SetViewport::Type:
        {
          const auto &cmd = reader.ReadCommand<Commands::SetViewport>();
          const auto &rt = renderTargets.Get(_state.CurrentRenderTarget);

          GLint vx = static_cast<GLint>(cmd.Position.x);
          GLint vy = static_cast<GLint>(rt.Height() - (cmd.Position.y + cmd.Size.y));
          GLsizei vw = static_cast<GLsizei>(cmd.Size.x);
          GLsizei vh = static_cast<GLsizei>(cmd.Size.y);
          glViewport(vx, vy, vw, vh);
          break;
        }
        case Commands::BindRenderTarget::Type:
        {
          const auto &cmd = reader.ReadCommand<Commands::BindRenderTarget>();
          if (cmd.RenderTarget == _state.CurrentRenderTarget)
          {
            break;
          }

          assert(cmd.RenderTarget.IsValid() && "Invalid render target handle in BindRenderTarget.");
          renderTargets.Bind(cmd.RenderTarget);
          _state.CurrentRenderTarget = cmd.RenderTarget;

          auto &rt = renderTargets.Get(cmd.RenderTarget);
          glViewport(0, 0, rt.Width(), rt.Height());

          break;
        }
        case Commands::ClearRenderTarget::Type:
        {
          const auto &cmd = reader.ReadCommand<Commands::ClearRenderTarget>();

          GLbitfield clearMask = MapBufferBitFlags(cmd.Clear);
          if (HasFlag(cmd.Clear, BufferBitFlags::Colour))
          {
            const auto &colour = cmd.Colour.ToVec4();
            glClearColor(colour.x, colour.y, colour.z, colour.w);
          }
          if (HasFlag(cmd.Clear, BufferBitFlags::Depth))
          {
            glClearDepthf(Maths::Clamp(cmd.Depth, 0.f, 1.f));
          }
          if (HasFlag(cmd.Clear, BufferBitFlags::Stencil))
          {
            glClearStencil(static_cast<GLint>(cmd.Stencil));
          }
          if (clearMask != 0)
          {
            glClear(clearMask);
          }

          break;
        }
        case Commands::BlitRenderTarget::Type:
        {
          const auto &cmd = reader.ReadCommand<Commands::BlitRenderTarget>();

          auto &srcRT = renderTargets.Get(cmd.Source);
          glBindFramebuffer(GL_READ_FRAMEBUFFER, srcRT.GetHandle());

          GLint srcX0 = static_cast<GLint>(cmd.SourcePosition.x);
          GLint srcY0 = static_cast<GLint>(srcRT.Height() - (cmd.SourcePosition.y + cmd.SourceSize.y));
          GLint srcX1 = static_cast<GLint>(cmd.SourcePosition.x + cmd.SourceSize.x);
          GLint srcY1 = static_cast<GLint>(srcRT.Height() - cmd.SourcePosition.y);

          auto &dstRT = renderTargets.Get(cmd.Destination);
          glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dstRT.GetHandle());

          GLint dstX0 = static_cast<GLint>(cmd.DestinationPosition.x);
          GLint dstY0 =
            static_cast<GLint>(dstRT.Height() - (cmd.DestinationPosition.y + cmd.DestinationSize.y));
          GLint dstX1 = static_cast<GLint>(cmd.DestinationPosition.x + cmd.DestinationSize.x);
          GLint dstY1 = static_cast<GLint>(dstRT.Height() - cmd.DestinationPosition.y);

          GLbitfield blitMask = MapBufferBitFlags(cmd.Mask);
          GLenum filter = MapFilterMode(cmd.Filter);
          glBlitFramebuffer(srcX0, srcY0, srcX1, srcY1, dstX0, dstY0, dstX1, dstY1, blitMask, filter);

          // Restore previous framebuffer
          renderTargets.Bind(_state.CurrentRenderTarget);
          break;
        }
        case Commands::CompositeRenderTarget::Type:
        {
          const auto &cmd = reader.ReadCommand<Commands::CompositeRenderTarget>();
          const auto &dstRT = renderTargets.Get(cmd.Destination);
          const auto &srcRT = renderTargets.Get(cmd.Source);

          renderTargets.Bind(cmd.Destination);
          glViewport(0, 0, dstRT.Width(), dstRT.Height());

          const auto &shader = shaders.Get(shaders.GetBuiltin(BuiltinShader::PostProcess_Passthrough));
          shader.Bind();
          shader.SetUniform("u_Texture", 0);
          glBindTextureUnit(0, srcRT.GetColourAttachment(0).Texture);

          auto &quad = meshes.Get(meshes.GetFullScreenQuad());
          quad.Bind();
          quad.Draw();

          break;
        }
        case Commands::CompositeRenderTargetWithOpacity::Type:
        {
          const auto &cmd = reader.ReadCommand<Commands::CompositeRenderTargetWithOpacity>();
          const auto &dstRT = renderTargets.Get(cmd.Destination);
          const auto &srcRT = renderTargets.Get(cmd.Source);

          renderTargets.Bind(cmd.Destination);
          glViewport(0, 0, dstRT.Width(), dstRT.Height());

          const auto &shader = shaders.Get(shaders.GetBuiltin(BuiltinShader::PostProcess_Passthrough));
          shader.Bind();
          shader.SetUniform("u_Texture", 0);
          glBindTextureUnit(0, srcRT.GetColourAttachment(0).Texture);

          glBlendFunc(GL_CONSTANT_COLOR, GL_ZERO);
          glBlendColor(cmd.Opacity, cmd.Opacity, cmd.Opacity, cmd.Opacity);

          auto &quad = meshes.Get(meshes.GetFullScreenQuad());
          quad.Bind();
          quad.Draw();

          glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

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
          DrawText(font, shader, cmd.Text, cmd.Colour, cmd.Position, cmd.FontSize);
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
    glEnable(GL_DEPTH_TEST);
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

  void Renderer::DrawText(Font &font, Shader &shader, utf8_stringview text,
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

    utf32_string result = Text::Decode(text, Text::utf8);
    auto count = result.size();

    while (count > 0)
    {
      auto batchSize = Maths::Min(count, static_cast<size_t>(GlyphVertex::BatchSize));
      utf32_stringview batch(result.data() + (result.size() - count), batchSize);
      count -= batchSize;

      _glyphVertices.clear();
      const auto &characters = font.Characters();
      for (const auto c : batch)
      {
        // Check for newline character
        if (c == '\n')
        {
          cursor.x = position.x;
          cursor.y += font.Metrics().Height * scale;
          continue;
        }

        const auto &glyph = characters.find(c);
        if (glyph == characters.end())
        {
          // TODO(fix): this check is better than before but we should default to using a missing glyph character
          //KRYS_WARN("Font '{}' does not contain glyph for character '{}'", font.Family().Id, c);
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