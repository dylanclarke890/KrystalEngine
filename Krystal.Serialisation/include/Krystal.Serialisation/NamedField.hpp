#pragma once

#include "Krystal.Lib/String.hpp"

namespace Krys::Serialisation
{
  template <typename T>
  struct NamedField
  {
    string Name;
    T &Value;

    constexpr NamedField(string name, T &value) : Name(name), Value(value)
    {
    }
  };
}