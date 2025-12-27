#pragma once

#include "Krystal.Gfx.OpenGL/gl.hpp"
#include "Krystal.Gfx.OpenGL/Resources/Shader.hpp"
#include "Krystal.Gfx.OpenGL/Utils.hpp"
#include "Krystal.Gfx/Resources/Font.hpp"
#include "Krystal.Lib/Macros.hpp"
#include "Krystal.Lib/Types/Map.hpp"
#include "Krystal.Lib/Types/Span.hpp"
#include "Krystal.Maths/Vector.hpp"

namespace Krys::Gfx::OpenGL
{
  class Font
  {
    NO_COPY(Font)

    FontType _type {FontType::Bitmap};
    FontFamilyHandle _fontFamily;
    TextureHandle _atlasTexture;
    CharacterMap _characters;
    Maths::Vec2u _atlasSize {0u};
    FontMetrics _metrics {};
    float _ptSize {0.f};
    SDFParams _sdfParams {};

    Font(FontType type, FontFamilyHandle fontFamily, TextureHandle atlasTexture,
         const FontAtlasData &data) noexcept
        : _type(type), _fontFamily(fontFamily), _atlasTexture(atlasTexture), _characters(data.Characters),
          _atlasSize(data.Size), _metrics(data.Metrics)
    {
    }

  public:
    ~Font() noexcept = default;

    MOVE_SWAP(Font)

    NO_DISCARD static Font BitmapAtlas(FontFamilyHandle fontFamily, TextureHandle texture,
                                       const FontAtlasData &data, float ptSize) noexcept
    {
      auto font = Font(FontType::Bitmap, fontFamily, texture, data);
      font._ptSize = ptSize;
      return font;
    }

    NO_DISCARD static Font SDFAtlas(FontFamilyHandle fontFamily, TextureHandle texture,
                                    const FontAtlasData &data,
                                    const SDFParams &sdfParams = SDFParams::Defaults()) noexcept
    {
      auto font = Font(FontType::SDF, fontFamily, texture, data);
      font._sdfParams = sdfParams;
      return font;
    }

    NO_DISCARD static Font MSDFAtlas(FontFamilyHandle fontFamily, TextureHandle texture,
                                     const FontAtlasData &data,
                                     const SDFParams &sdfParams = SDFParams::Defaults()) noexcept
    {
      auto font = Font(FontType::MSDF, fontFamily, texture, data);
      font._sdfParams = sdfParams;
      return font;
    }

    NO_DISCARD static Font MTSDFAtlas(FontFamilyHandle fontFamily, TextureHandle texture,
                                      const FontAtlasData &data,
                                      const SDFParams &sdfParams = SDFParams::Defaults()) noexcept
    {
      auto font = Font(FontType::MTSDF, fontFamily, texture, data);
      font._sdfParams = sdfParams;
      return font;
    }

    NO_DISCARD FontType Type() const noexcept
    {
      return _type;
    }

    NO_DISCARD FontFamilyHandle Family() const noexcept
    {
      return _fontFamily;
    }

    NO_DISCARD TextureHandle AtlasTexture() const noexcept
    {
      return _atlasTexture;
    }

    NO_DISCARD float PtSize() const noexcept
    {
      return _ptSize;
    }

    NO_DISCARD const SDFParams &SDFParams() const noexcept
    {
      return _sdfParams;
    }

    NO_DISCARD const CharacterMap &Characters() const noexcept
    {
      return _characters;
    }

    NO_DISCARD const Maths::Vec2u &AtlasSize() const noexcept
    {
      return _atlasSize;
    }

    NO_DISCARD const FontMetrics &Metrics() const noexcept
    {
      return _metrics;
    }

  private:
    void Swap(Font &other) noexcept
    {
      std::swap(other._type, _type);
      std::swap(other._fontFamily, _fontFamily);
      std::swap(other._atlasTexture, _atlasTexture);
      std::swap(other._ptSize, _ptSize);
      std::swap(other._sdfParams, _sdfParams);
      std::swap(other._characters, _characters);
      std::swap(other._atlasSize, _atlasSize);
      std::swap(other._metrics, _metrics);
    }
  };
}