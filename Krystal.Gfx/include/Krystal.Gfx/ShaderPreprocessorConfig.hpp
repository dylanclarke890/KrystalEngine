#pragma once

#include "Krystal.Lib/Map.hpp"
#include "Krystal.Lib/String.hpp"

namespace Krys::Gfx
{
  struct ShaderPreprocessorConfig
  {
    Map<string, string> Defines {};

    void Define(const string &key, const string &value) noexcept
    {
      Defines[key] = value;
    }
  };
}