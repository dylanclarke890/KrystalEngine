#pragma once

#include "Krystal.Gfx.OpenGL/gl.hpp"
#include "Krystal.Gfx.OpenGL/Resources/Shader.hpp"
#include "Krystal.Gfx.OpenGL/Utils.hpp"
#include "Krystal.Gfx/Resources/Font.hpp"
#include "Krystal.Lib/Mixins/NonCopyable.hpp"
#include "Krystal.Lib/Types/Map.hpp"
#include "Krystal.Lib/Types/Span.hpp"
#include "Krystal.Maths/Vector.hpp"

namespace Krys::Gfx::OpenGL
{
  class Font : NonCopyable<Font>
  {
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

    Font(Font &&other) noexcept
        : _type(std::exchange(other._type, FontType::Bitmap)),
          _fontFamily(std::exchange(other._fontFamily, {})),
          _atlasTexture(std::exchange(other._atlasTexture, {})), _characters(std::move(other._characters)),
          _atlasSize(std::exchange(other._atlasSize, {})), _metrics(std::exchange(other._metrics, {})),
          _ptSize(std::exchange(other._ptSize, 0.f)),
          _sdfParams(std::exchange(other._sdfParams, SDFParams::Defaults()))
    {
    }

    Font &operator=(Font &&other) noexcept
    {
      if (this != &other)
      {
        _type = std::exchange(other._type, FontType::Bitmap);
        _fontFamily = std::exchange(other._fontFamily, {});
        _atlasTexture = std::exchange(other._atlasTexture, {});
        _characters = std::move(other._characters);
        _atlasSize = std::exchange(other._atlasSize, {});
        _metrics = std::exchange(other._metrics, {});
        _ptSize = std::exchange(other._ptSize, 0.f);
        _sdfParams = std::exchange(other._sdfParams, SDFParams::Defaults());
      }
      return *this;
    }

    KRYS_NODISCARD static Font BitmapAtlas(FontFamilyHandle fontFamily, TextureHandle texture,
                                           const FontAtlasData &data, float ptSize) noexcept
    {
      auto font = Font(FontType::Bitmap, fontFamily, texture, data);
      font._ptSize = ptSize;
      return font;
    }

    KRYS_NODISCARD static Font SDFAtlas(FontFamilyHandle fontFamily, TextureHandle texture,
                                        const FontAtlasData &data,
                                        const SDFParams &sdfParams = SDFParams::Defaults()) noexcept
    {
      auto font = Font(FontType::SDF, fontFamily, texture, data);
      font._sdfParams = sdfParams;
      return font;
    }

    KRYS_NODISCARD static Font MSDFAtlas(FontFamilyHandle fontFamily, TextureHandle texture,
                                         const FontAtlasData &data,
                                         const SDFParams &sdfParams = SDFParams::Defaults()) noexcept
    {
      auto font = Font(FontType::MSDF, fontFamily, texture, data);
      font._sdfParams = sdfParams;
      return font;
    }

    KRYS_NODISCARD static Font MTSDFAtlas(FontFamilyHandle fontFamily, TextureHandle texture,
                                          const FontAtlasData &data,
                                          const SDFParams &sdfParams = SDFParams::Defaults()) noexcept
    {
      auto font = Font(FontType::MTSDF, fontFamily, texture, data);
      font._sdfParams = sdfParams;
      return font;
    }

    KRYS_NODISCARD FontType Type() const noexcept
    {
      return _type;
    }

    KRYS_NODISCARD FontFamilyHandle Family() const noexcept
    {
      return _fontFamily;
    }

    KRYS_NODISCARD TextureHandle AtlasTexture() const noexcept
    {
      return _atlasTexture;
    }

    KRYS_NODISCARD float PtSize() const noexcept
    {
      return _ptSize;
    }

    KRYS_NODISCARD const SDFParams &SDFParams() const noexcept
    {
      return _sdfParams;
    }

    KRYS_NODISCARD const CharacterMap &Characters() const noexcept
    {
      return _characters;
    }

    KRYS_NODISCARD const Maths::Vec2u &AtlasSize() const noexcept
    {
      return _atlasSize;
    }

    KRYS_NODISCARD const FontMetrics &Metrics() const noexcept
    {
      return _metrics;
    }
  };
}