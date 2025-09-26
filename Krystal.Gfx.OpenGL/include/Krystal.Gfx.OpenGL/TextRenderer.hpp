#pragma once

#include "Krystal.Gfx.OpenGL/Fonts/FontRegistry.hpp"
#include "Krystal.Gfx.OpenGL/Shaders/ShaderRegistry.hpp"
#include "Krystal.Gfx/Colour.hpp"
#include "Krystal.Gfx/Handle.hpp"
#include "Krystal.Gfx/ResourceHandleCache.hpp"
#include "Krystal.Gfx/TextShaderDesc.hpp"
#include "Krystal.Lib/Macros.hpp"
#include "Krystal.Lib/Map.hpp"
#include "Krystal.Lib/String.hpp"
#include "Krystal.Maths/Vector.hpp"

namespace Krys::Gfx::OpenGL
{
  class TextRenderer
  {
    NO_COPY_MOVE(TextRenderer)

  private:
    FontRegistry &_fonts;
    ShaderRegistry &_shaders;
    int _dpi;
    Map<TextShaderDesc, ShaderHandle> _fontShaders;

  public:
    TextRenderer(FontRegistry &fonts, ShaderRegistry &shaders, int dpi) noexcept
        : _fonts(fonts), _shaders(shaders), _dpi(dpi)
    {
    }

    void Draw(const string &text, FontHandle fontHandle, const Maths::Vec2 &position,
                    const Colour &colour = Colours::Black) noexcept
    {
      auto &font = _fonts.Get(fontHandle);
      auto &shader = GetOrAdd({.FontType = font.Type()});

      shader.Bind();
      shader.SetUniform("u_TextColor", colour.ToVec3());

      float scale = 1.0f;
      if (font.Type() != FontType::Bitmap)
      {
        scale = _dpi / 72.0f;
        shader.SetUniform("u_PixelRange", font.SDFParams().PixelRange);
        shader.SetUniform("u_AtlasSize", Maths::Vec2(font.Atlas().AtlasSize));
      }

      font.DrawText(text, position, scale);
    }

    Shader &GetOrAdd(const TextShaderDesc &desc) noexcept
    {
      if (const auto it = _fontShaders.find(desc); it != _fontShaders.end())
      {
        return _shaders.Get(it->second);
      }

      const auto vertexShader = IO::Path("data/shaders/opengl/text-shader.vert");
      const auto fragmentShader = IO::Path("data/shaders/opengl/text-shader.frag");
      auto shader = _shaders.LoadTextShader(vertexShader, fragmentShader, desc);

      _fontShaders[desc] = shader;
      return _shaders.Get(shader);
    }
  };
}