#pragma once

#include "Krystal.Lib/String.hpp"

namespace Krys::Serialisation
{
  template <typename T>
  struct NamedField
  {
    stringview Name;
    T &Value;

    constexpr NamedField(stringview name, T &value) : Name(name), Value(value)
    {
    }
  };
}