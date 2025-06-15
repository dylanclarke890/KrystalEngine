#pragma once

#include "Krystal.Core/Core.hpp"

namespace Krys::Serialisation
{
  /// @brief Hook for serialisation of custom objects/types.
  template <typename Archive, typename T>
  void Save(Archive &, const T &) noexcept
  {
    static_assert(false, "You must provide a custom 'Save' function for type.");
  }

  /// @brief Hook for deserialisation of custom objects/types.
  template <typename Archive, typename T>
  void Load(Archive &, T &) noexcept
  {
    static_assert(false, "You must provide a custom 'Load' function for type.");
  }

  /// @brief Serialisation of arithmetic types.
  template <typename Archive, Arithmetic T>
  void Save(Archive &archive, const T &value) noexcept
  {
    archive(value);
  }

  /// @brief Deserialisation of arithmetic types.
  template <typename Archive, Arithmetic T>
  void Load(Archive &archive, T &value) noexcept
  {
    archive(value);
  }

  /// @brief Serialisation of byte values.
  template <typename Archive>
  void Save(Archive &archive, const byte &value) noexcept
  {
    archive(value);
  }

  /// @brief Deserialisation of byte values.
  template <typename Archive>
  void Load(Archive &archive, byte &value) noexcept
  {
    archive(value);
  }

  /// @brief Serialisation of boolean values.
  template <typename Archive>
  void Save(Archive &archive, const string &value) noexcept
  {
    archive(value);
  }

  /// @brief Deserialisation of boolean values.
  template <typename Archive>
  void Load(Archive &archive, string &value) noexcept
  {
    archive(value);
  }
}