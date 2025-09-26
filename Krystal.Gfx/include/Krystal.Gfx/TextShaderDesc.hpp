#pragma once

#include "Krystal.Gfx/Common.hpp"
#include "Krystal.Lib/HashUtils.hpp"
#include <type_traits>

namespace Krys::Gfx
{
  struct TextShaderDesc
  {
    FontType FontType {};
    bool EnableOutline {false};
    bool EnableShadow {false};
    bool EnableGlow {false};

    bool operator==(const TextShaderDesc &other) const noexcept
    {
      return FontType == other.FontType && EnableOutline == other.EnableOutline &&
             EnableShadow == other.EnableShadow && EnableGlow == other.EnableGlow;
    }
  };
}

namespace std
{
  template <>
  struct hash<Krys::Gfx::TextShaderDesc>
  {
    size_t operator()(const Krys::Gfx::TextShaderDesc &desc) const noexcept
    {
      return Krys::HashUtils::HashCombine(desc.FontType, desc.EnableOutline, desc.EnableShadow,
                                          desc.EnableGlow);
    }
  };
}