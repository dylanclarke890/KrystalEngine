#pragma once

#include "Krystal.Gfx/Resources/Font.hpp"
#include "Krystal.Lib/HashUtils.hpp"
#include "Krystal.Lib/Macros.hpp"
#include "Krystal.Lib/Types/Map.hpp"
#include "Krystal.Lib/String/String.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"
#include <type_traits>

namespace Krys::Gfx
{
  struct TextShaderDesc
  {
    FontType FontType {};
    bool EnableOutline {false};

    bool operator==(const TextShaderDesc &other) const noexcept
    {
      return FontType == other.FontType && EnableOutline == other.EnableOutline;
    }
  };

  struct ShaderPreprocessorConfig
  {
    Map<string, string> Defines {};

    void Define(const string &key, const string &value) noexcept
    {
      Defines[key] = value;
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
      return Krys::HashUtils::HashCombine(desc.FontType, desc.EnableOutline);
    }
  };
}