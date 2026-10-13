#pragma once

#include "Krystal.Core/Attributes.hpp"
#include <algorithm>
#include <compare>

namespace krys::boo::style
{
  struct DynamicRangeLimit;
}

namespace krys::boo
{
  /// @brief A `dynamic-range-limit` as a single value in [0, 1]: 0 is `standard`, 0.5 `constrained` and 1
  /// `no-limit`.
  class PlatformDynamicRangeLimit
  {
    friend style::DynamicRangeLimit;

  public:
    using ValueType = float;

  private:
    constexpr static ValueType StandardValue = 0;
    constexpr static ValueType ConstrainedValue = 0.5;
    constexpr static ValueType NoLimitValue = 1;

    ValueType _value {NoLimitValue};

  public:
    KRYS_NODISCARD constexpr static PlatformDynamicRangeLimit Standard() noexcept
    {
      return PlatformDynamicRangeLimit(StandardValue);
    }

    KRYS_NODISCARD constexpr static PlatformDynamicRangeLimit Constrained() noexcept
    {
      return PlatformDynamicRangeLimit(ConstrainedValue);
    }

    KRYS_NODISCARD constexpr static PlatformDynamicRangeLimit NoLimit() noexcept
    {
      return PlatformDynamicRangeLimit(NoLimitValue);
    }

    KRYS_NODISCARD constexpr static PlatformDynamicRangeLimit InitialValue() noexcept
    {
      return NoLimit();
    }

    KRYS_NODISCARD constexpr static PlatformDynamicRangeLimit InitialValueForVideos() noexcept
    {
      return NoLimit();
    }

    KRYS_NODISCARD constexpr static PlatformDynamicRangeLimit DefaultWhenSuppressingHDR() noexcept
    {
      return Standard();
    }

    KRYS_NODISCARD constexpr static PlatformDynamicRangeLimit DefaultWhenSuppressingHDRInVideos() noexcept
    {
      return Constrained();
    }

    // `dynamic-range-limit` mapped to PlatformDynamicRangeLimit.Value():
    // ["standard", "constrained"] -> [Standard().Value(), Constrained().Value()],
    // ["constrained", "noLimit"] -> [Constrained().Value(), NoLimit().Value()]
    KRYS_NODISCARD constexpr ValueType Value() const noexcept
    {
      return _value;
    }

    KRYS_NODISCARD constexpr auto operator<=>(const PlatformDynamicRangeLimit &) const noexcept = default;

  private:
    explicit constexpr PlatformDynamicRangeLimit(ValueType value) noexcept
        : _value(std::clamp(value, 0.0f, 1.0f))
    {
    }

    PlatformDynamicRangeLimit(float standardPercent, float constrainedPercent, float noLimitPercent) noexcept
        : _value(NormalizedAverage(standardPercent, constrainedPercent, noLimitPercent))
    {
    }

    KRYS_NODISCARD static ValueType NormalizedAverage(float standardPercent, float constrainedPercent,
                                                      float noLimitPercent) noexcept;
  };
}
