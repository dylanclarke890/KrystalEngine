#pragma once

#include "Krystal.Gfx/Handle.hpp"
#include "Krystal.IO/Path.hpp"
#include "Krystal.Lib/Allocators/StringRef.hpp"
#include "Krystal.Lib/Attributes.hpp"
#include "Krystal.Lib/List.hpp"
#include "Krystal.Lib/Macros.hpp"
#include "Krystal.Lib/Map.hpp"
#include "Krystal.Lib/Types.hpp"
#include "Krystal.Maths/Vector.hpp"
#include <compare>
#include <type_traits>

namespace Krys::Gfx
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

    NO_DISCARD auto operator<=>(const FontDesc &other) const noexcept = default;
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
    Maths::Vec2u Size {};    // Size of glyph
    Maths::Vec2i Bearing {}; // Offset from baseline to left/top of glyph
    int32 Advance {0u};      // Offset to advance to next glyph
    Maths::Vec2 UVMin;       // (u0, v0)
    Maths::Vec2 UVMax;       // (u1, v1)
  };

  struct FontAtlasData
  {
    Maths::Vec2u Size;
    List<uint8> Pixels;
    Map<uchar, Character> Characters;
  };
}

namespace std
{
  template <>
  struct hash<Krys::Gfx::FontDesc>
  {
    size_t operator()(const Krys::Gfx::FontDesc &desc) const noexcept
    {
      size_t h1 = std::hash<Krys::uint8>()(static_cast<Krys::uint8>(desc.Type));
      size_t h2 = std::hash<float>()(desc.Size);
      return h1 ^ (h2 << 1);
    }
  };
}

namespace Krys::Gfx
{
  class FontFamily
  {
    NO_COPY(FontFamily)

  private:
    StringRef _name;
    IO::Path _path;

  public:
    explicit FontFamily(StringRef name, const IO::Path &path) noexcept : _name(name), _path(path)
    {
    }

    ~FontFamily() = default;

    MOVE_SWAP(FontFamily)

    NO_DISCARD StringRef Name() const noexcept
    {
      return _name;
    }

    NO_DISCARD const IO::Path &Path() const noexcept
    {
      return _path;
    }

  private:
    void Swap(FontFamily &other) noexcept
    {
      _path = std::move(other._path);
    }
  };
}