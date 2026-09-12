#pragma once

#include "Krystal.Core/Hash.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/String.hpp"
#include "Krystal.Gfx/Resources/Font.hpp"
#include <type_traits>

namespace krys::Gfx
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
    HashMap<string, string> Defines {};

    void Define(const string &key, const string &value) noexcept
    {
      Defines[key] = value;
    }
  };
}

namespace std
{
  template <>
  struct hash<krys::Gfx::TextShaderDesc>
  {
    size_t operator()(const krys::Gfx::TextShaderDesc &desc) const noexcept
    {
      return krys::Hash::Combine(desc.FontType, desc.EnableOutline);
    }
  };
}