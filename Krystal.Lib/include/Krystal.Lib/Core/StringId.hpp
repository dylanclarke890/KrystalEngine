#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Core/Hash.hpp"
#include "Krystal.Lib/String/String.hpp"
#include <xhash>

namespace Krys
{
#define KRYS_SID(s) (Krys::StringId {Krys::operator""_sid(s, sizeof(s) - 1)})

  /// @brief Represents a hashed string.
  class StringId
  {
  private:
    uint32 _hash;

  public:
    /// @brief Constructs a `StringId` with the given hash.
    /// @param hash The hashed string.
    explicit constexpr StringId(uint32 hash) noexcept : _hash(hash)
    {
    }

    /// @brief Constructs a `StringId` with the given string to hash.
    /// @param text The string to hash.
    explicit constexpr StringId(const string &text) noexcept : _hash(fnv1a32(text.c_str(), text.size()))
    {
    }

    /// @brief Constructs a `StringId` with the given string to hash.
    /// @param text The string to hash.
    /// @param size The size of the string to hash.
    constexpr StringId(const char *text, size_t size) noexcept : _hash(fnv1a32(text, size))
    {
    }

    /// @brief Compares this `StringId` with another for equality.
    KRYS_NODISCARD constexpr bool operator==(const StringId &other) const noexcept
    {
      return _hash == other._hash;
    }

    /// @brief Compares this `StringId` with another for inequality.
    KRYS_NODISCARD constexpr bool operator!=(const StringId &other) const noexcept
    {
      return !(*this == other);
    }

    /// @brief Returns the hash value of the string id.
    KRYS_NODISCARD constexpr operator uint32() const noexcept
    {
      return _hash;
    }
  };

  /// @brief Hashes a string literal at compile time.
  KRYS_NODISCARD constexpr uint32 operator""_sid(char const *s, size_t count) noexcept
  {
    return StringId(s, count);
  }
}

namespace std
{
  /// @brief Hash function for `Krys::StringId`.
  template <>
  struct hash<Krys::StringId>
  {
    /// @brief Computes the hash of a `Krys::StringId`.
    /// @param id The `Krys::StringId` to hash.
    /// @return The hash value.
    KRYS_NODISCARD size_t operator()(const Krys::StringId &id) const noexcept
    {
      return static_cast<size_t>(id);
    }
  };
}