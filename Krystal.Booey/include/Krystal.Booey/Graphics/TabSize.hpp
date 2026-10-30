#pragma once

#include "Krystal.Core/Attributes.hpp"

namespace krys::boo
{
  enum TabSizeValueType
  {
    LengthValueType,
    SpaceValueType,
  };

  /// @brief A used `tab-size`: a number of spaces or a length in pixels.
  class TabSize
  {
  private:
    float _value {0};
    bool _isSpaces {false};

  public:
    constexpr TabSize() noexcept = default;

    constexpr TabSize(float numOrLength, TabSizeValueType isSpaces = SpaceValueType) noexcept
        : _value(numOrLength), _isSpaces(isSpaces)
    {
    }

    KRYS_NODISCARD constexpr bool IsSpaces() const noexcept
    {
      return _isSpaces;
    }

    KRYS_NODISCARD constexpr float WidthInPixels(float spaceWidth) const noexcept
    {
      return _isSpaces ? _value * spaceWidth : _value;
    }

    KRYS_NODISCARD constexpr float Value() const noexcept
    {
      return _value;
    }

    KRYS_NODISCARD constexpr explicit operator bool() const noexcept
    {
      return _value;
    }

    KRYS_NODISCARD constexpr bool operator==(const TabSize &) const noexcept = default;
  };
}
