#pragma once

#include "Krystal.Lib/String.hpp"

namespace Krys::Serialisation
{
  template <typename T>
  struct NamedField
  {
    stringview Name;
    T &Value;

    constexpr NamedField(stringview name, T &value) noexcept : Name(name), Value(value)
    {
    }
  };

#define KRYS_NAMED_FIELD(value) ::Krys::Serialisation::NamedField<decltype(value)>(#value, value)
}