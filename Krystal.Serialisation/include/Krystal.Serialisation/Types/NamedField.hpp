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
  void Save(Archive &ar, const NamedField<T> &f) noexcept
  {
    string name(f.Name);
    Save(ar, name);
    Save(ar, f.Value);
  }

  template <typename Archive, typename T>
  void Load(Archive &ar, const NamedField<T> &f) noexcept
  {
    string nameFromStream;
    Load(ar, nameFromStream);
    Load(ar, const_cast<T &>(f.Value));
  }

#define KRYS_NAMED_FIELD(value) ::Krys::Serialisation::NamedField<decltype(value)>(#value, value)
}