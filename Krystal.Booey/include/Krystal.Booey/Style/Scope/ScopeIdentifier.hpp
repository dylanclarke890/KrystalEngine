#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Numeric.hpp"
#include <functional>

namespace krys::boo::style
{
  /// @brief Identifies a style scope (a document's or a shadow root's) uniquely while the process runs.
  class ScopeIdentifier
  {
  private:
    uint64 _value {0};

    explicit constexpr ScopeIdentifier(uint64 value) noexcept : _value(value)
    {
    }

  public:
    constexpr ScopeIdentifier() noexcept = default;

    KRYS_NODISCARD static ScopeIdentifier Generate() noexcept;

    KRYS_NODISCARD constexpr uint64 ToUInt64() const noexcept
    {
      return _value;
    }

    KRYS_NODISCARD constexpr bool operator==(const ScopeIdentifier &) const noexcept = default;
  };
}

namespace std
{
  template <>
  struct hash<krys::boo::style::ScopeIdentifier>
  {
    size_t operator()(krys::boo::style::ScopeIdentifier identifier) const noexcept
    {
      return hash<krys::uint64> {}(identifier.ToUInt64());
    }
  };
}
