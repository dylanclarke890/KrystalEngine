#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Hash.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/StringAtom.hpp"

namespace krys::boo
{
  /// @brief The used value of `font-palette`.
  struct FontPalette
  {
    enum class Type : uint8
    {
      Normal,
      Light,
      Dark,
      Custom
    };

    Type PaletteType {Type::Normal};
    StringAtom Identifier {StringAtom::Null()};

    KRYS_NODISCARD bool operator==(const FontPalette &other) const noexcept
    {
      if (PaletteType == Type::Custom)
      {
        return other.PaletteType == Type::Custom && Identifier == other.Identifier;
      }
      return PaletteType == other.PaletteType;
    }
  };
}

namespace krys
{
  template <>
  struct DefaultHash<boo::FontPalette>
  {
    KRYS_NODISCARD static size_t Hash(const boo::FontPalette &palette) noexcept
    {
      if (palette.PaletteType == boo::FontPalette::Type::Custom)
      {
        return krys::Hash::Combine(static_cast<uint8>(palette.PaletteType),
                                   std::hash<StringAtom> {}(palette.Identifier));
      }
      return DefaultHash<uint8>::Hash(static_cast<uint8>(palette.PaletteType));
    }

    KRYS_NODISCARD static bool Equal(const boo::FontPalette &a, const boo::FontPalette &b) noexcept
    {
      return a == b;
    }
  };
}
