#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Maths/Vector.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/List.hpp"

namespace krys::gfx
{
  /// @brief How a font atlas encodes glyphs.
  enum class FontType : uint8
  {
    Bitmap,
    SDF,
    MSDF,
    MTSDF
  };

  /// @brief How distance field atlases are generated.
  struct SDFParams
  {
    float EMSizeInPixels {0.f};
    float PixelRange {0.f};
    float MiterLimit {0.f};

    KRYS_NODISCARD constexpr static SDFParams Defaults() noexcept
    {
      return {
        .EMSizeInPixels = 64.f,
        .PixelRange = 12.5f,
        .MiterLimit = 1.f,
      };
    }
  };

  /// @brief A glyph's metrics and its place in the atlas.
  struct Character
  {
    /// @brief The size of the glyph.
    Vec2u Size {};
    /// @brief The offset from the baseline to the left and top of the glyph.
    Vec2i Bearing {};
    /// @brief The offset to the next glyph.
    int32 Advance {0};
    Vec2 UVMin {};
    Vec2 UVMax {};
  };

  using CharacterMap = HashMap<char32, Character>;

  struct FontMetrics
  {
    /// @brief The distance from the baseline to the highest ascender.
    float Ascender {0.f};
    /// @brief The distance from the baseline to the lowest descender (usually negative).
    float Descender {0.f};
    /// @brief The smallest distance between lines of text.
    float Height {0.f};
    /// @brief The typographical distance between baselines of consecutive lines of text.
    float LineHeight {0.f};
  };

  /// @brief A font's glyphs rasterised into one image.
  struct FontAtlasData
  {
    Vec2u Size {};
    List<uint8> Pixels {};
    CharacterMap Characters {};
    FontMetrics Metrics {};
  };
}
