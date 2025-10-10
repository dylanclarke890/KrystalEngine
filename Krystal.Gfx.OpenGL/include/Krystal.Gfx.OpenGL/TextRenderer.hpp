#pragma once

#include "Krystal.Gfx.OpenGL/Registries/FontRegistry.hpp"
#include "Krystal.Gfx.OpenGL/Registries/ShaderRegistry.hpp"
#include "Krystal.Gfx/Colour.hpp"
#include "Krystal.Gfx/Handle.hpp"
#include "Krystal.Gfx/ResourceHandleCache.hpp"
#include "Krystal.Gfx/TextShaderDesc.hpp"
#include "Krystal.Lib/Macros.hpp"
#include "Krystal.Lib/Map.hpp"
#include "Krystal.Lib/String.hpp"
#include "Krystal.Maths/Vector.hpp"
#include <cassert>

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
      Font &font = _fonts.Get(fontHandle);
      Shader &shader = GetOrAdd({.FontType = font.Type()});

      shader.Bind();
      shader.SetUniform("u_TextColor", colour.ToVec3());

      float scale = 1.0f;
      if (font.Type() != FontType::Bitmap)
      {
        scale = _dpi / 72.0f;
        SetSDFParams(shader, font);
      }

      font.DrawText(text, position, scale);
    }

    void DrawOutlined(const string &text, FontHandle fontHandle, const Maths::Vec2 &position,
                      const Colour &textColour = Colours::Black, const Colour &outlineColour = Colours::White,
                      float outlineWidth = 3.f) noexcept
    {
      Font &font = _fonts.Get(fontHandle);
      Shader &shader = GetOrAdd({.FontType = font.Type(), .EnableOutline = true});

      assert(font.Type() != FontType::Bitmap && "Outlined text is not supported for bitmap fonts.");

      shader.Bind();
      shader.SetUniform("u_TextColor", textColour.ToVec3());

      float scale = _dpi / 72.0f;
      SetSDFParams(shader, font);

      shader.SetUniform("u_OutlineColor", outlineColour.ToVec3());
      shader.SetUniform("u_OutlineWidthAbsolute", outlineWidth / 3.f);
      shader.SetUniform("u_OutlineWidthRelative", outlineWidth / 20.f);
      shader.SetUniform("u_Threshold", 0.5f);

      font.DrawText(text, position, scale);
    }

    Shader &GetOrAdd(const TextShaderDesc &desc) noexcept
    {
      if (const auto it = _fontShaders.find(desc); it != _fontShaders.end())
      {
        return _shaders.Get(it->second);
      }

      const auto vertexShader = IO::Path("text-shader.vert");
      const auto fragmentShader = IO::Path("text-shader.frag");
      auto shader = _shaders.LoadTextShader(vertexShader, fragmentShader, desc);

      _fontShaders[desc] = shader;
      return _shaders.Get(shader);
    }

    void SetSDFParams(Shader &shader, Font &font) noexcept
    {
      Maths::Vec2 unitRange = Maths::Vec2(font.SDFParams().PixelRange) / Maths::Vec2(font.Atlas().AtlasSize);
      shader.SetUniform("u_UnitRange", unitRange);
    }
  };
}