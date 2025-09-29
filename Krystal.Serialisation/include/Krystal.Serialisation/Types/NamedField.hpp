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

  template <typename Archive, typename T>
  void Save(Archive &archive, const NamedField<T> &field) noexcept
  {
    archive(pair.first);
    archive(pair.second);
  }

  template <typename Archive, typename T>
  void Load(Archive &archive, const NamedField<T> &field) noexcept
  {
    archive(pair.first);
    archive(pair.second);
  }

#define KRYS_NAMED_FIELD(value) ::Krys::Serialisation::NamedField<decltype(value)>(#value, value)
}