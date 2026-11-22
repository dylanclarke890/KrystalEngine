#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Hashing/FastHash.hpp"
#include "Krystal.Core/Hashing/StableHash.hpp"
#include "Krystal.Core/Strings/String.hpp"
#include <xhash>

namespace krys
{
#define KRYS_SID(s) (krys::StringId {krys::operator""_sid(s, sizeof(s) - 1)})

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
    explicit constexpr StringId(const string &text) noexcept : _hash(Fnv1a32(text))
    {
    }

    /// @brief Constructs a `StringId` with the given string to hash.
    /// @param text The string to hash.
    /// @param size The size of the string to hash.
    constexpr StringId(const char *text, size_t size) noexcept : _hash(Fnv1a32(stringview(text, size)))
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
  KRYS_NODISCARD constexpr uint32 operator""_sid(const char *s, size_t count) noexcept
  {
    return StringId(s, count);
  }

  template <>
  struct FastHash<StringId>
  {
    KRYS_NODISCARD size_t operator()(StringId id) const noexcept
    {
      return FastHash<uint32> {}(static_cast<uint32>(id));
    }
  };
}
