#pragma once

#include "Krystal.Gfx/Handle.hpp"
#include "Krystal.IO/Path.hpp"
#include "Krystal.Lib/Macros.hpp"
#include "Krystal.Lib/Map.hpp"
#include "Krystal.Lib/Types.hpp"
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
    float Size {16.0f};

    NO_DISCARD auto operator<=>(const FontDesc &other) const noexcept = default;
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
    IO::Path _path;

  public:
    explicit FontFamily(const IO::Path &path) noexcept : _path(path)
    {
    }

    ~FontFamily() = default;

    MOVE_SWAP(FontFamily)

    const IO::Path &Path() const noexcept
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