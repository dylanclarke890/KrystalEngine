#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/IO/Path.hpp"
#include "Krystal.Core/Maths/Vector.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/StronglyTypedValue.hpp"
#include "Krystal.Core/Utils/Move.hpp"
#include "Krystal.Gfx/Handle.hpp"
#include "Krystal.Gfx/Vertex.hpp"
#include <compare>
#include <type_traits>

namespace krys::Gfx
{
  enum class FontType : uint8
  {
    Bitmap,
    SDF,
    MSDF,
    MTSDF
  };

  struct FontDesc
  {
    FontFamilyHandle Family;
    FontType Type {FontType::Bitmap};
    float Size {16.f};

    KRYS_NODISCARD auto operator<=>(const FontDesc &other) const noexcept = default;
  };

  struct SDFParams
  {
    float EMSizeInPixels {0.f};
    float PixelRange {0.f};
    float MiterLimit {0.f};

    constexpr static SDFParams Defaults() noexcept
    {
      return {
        .EMSizeInPixels = 64.f,
        .PixelRange = 12.5f,
        .MiterLimit = 1.f,
      };
    }
  };

  struct Character
  {
    Vec2u Size {};      // Size of glyph
    Vec2i Bearing {};   // Offset from baseline to left/top of glyph
    int32 Advance {0u}; // Offset to advance to next glyph
    Vec2 UVMin;         // (u0, v0)
    Vec2 UVMax;         // (u1, v1)
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

  struct FontAtlasData
  {
    Vec2u Size;
    List<uint8> Pixels;
    CharacterMap Characters;
    FontMetrics Metrics;
  };
}

namespace std
{
  template <>
  struct hash<krys::Gfx::FontDesc>
  {
    size_t operator()(const krys::Gfx::FontDesc &desc) const noexcept
    {
      size_t h1 = std::hash<krys::uint8>()(static_cast<krys::uint8>(desc.Type));
      size_t h2 = std::hash<float>()(desc.Size);
      return h1 ^ (h2 << 1);
    }
  };
}

namespace krys::Gfx
{
  class FontFamily
  {
    KRYS_NON_COPYABLE(FontFamily);

  private:
    utf8_string _name;
    io::Path _path;
    List<FontHandle> _fonts;

  public:
    FontFamily(utf8_string name, const io::Path &path) noexcept : _name(name), _path(path)
    {
    }

    ~FontFamily() = default;

    FontFamily(FontFamily &&other) noexcept
        : _name(std::exchange(other._name, utf8_string {})), _path(std::exchange(other._path, io::Path {})),
          _fonts(krys::move(other._fonts))
    {
    }

    FontFamily &operator=(FontFamily &&other) noexcept
    {
      if (this != &other)
      {
        _name = std::exchange(other._name, utf8_string {});
        _path = std::exchange(other._path, io::Path {});
        _fonts = krys::move(other._fonts);
      }
      return *this;
    }

    KRYS_NODISCARD utf8_stringview Name() const noexcept
    {
      return _name;
    }

    KRYS_NODISCARD const io::Path &Path() const noexcept
    {
      return _path;
    }

    void AddFont(FontHandle font)
    {
      _fonts.push_back(font);
    }

    void RemoveFont(FontHandle font)
    {
      std::erase(_fonts, font);
    }

    KRYS_NODISCARD const List<FontHandle> &Fonts() const noexcept
    {
      return _fonts;
    }
  };
}