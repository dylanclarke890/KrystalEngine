#pragma once

#include "Krystal.Lib/String.hpp"

namespace Krys::Serialisation
{
  /// @brief Serialisation of string values.
  template <typename Archive>
  void Save(Archive &archive, const string &value) noexcept
  {
    archive(value);
  }

  /// @brief Deserialisation of string values.
  template <typename Archive>
  void Load(Archive &archive, string &value) noexcept
  {
    archive(value);
  }
}