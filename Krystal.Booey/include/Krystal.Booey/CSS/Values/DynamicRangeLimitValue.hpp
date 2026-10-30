#pragma once

#include "Krystal.Booey/CSS/Values/Color/DynamicRangeLimit.hpp"
#include "Krystal.Booey/CSS/Values/Value.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  /// @brief A `<'dynamic-range-limit'>` value.
  class DynamicRangeLimitValue final : public Value
  {
  private:
    css::DynamicRangeLimit _dynamicRangeLimit;

    explicit DynamicRangeLimitValue(css::DynamicRangeLimit &&dynamicRangeLimit) noexcept;

  public:
    KRYS_NODISCARD static Ref<DynamicRangeLimitValue>
      Create(css::DynamicRangeLimit dynamicRangeLimit) noexcept;

    KRYS_NODISCARD const css::DynamicRangeLimit &GetDynamicRangeLimit() const noexcept
    {
      return _dynamicRangeLimit;
    }

    KRYS_NODISCARD CSSOMString CustomCSSText(const SerialisationContext &context) const noexcept;
    KRYS_NODISCARD bool Equals(const DynamicRangeLimitValue &other) const noexcept;
    KRYS_NODISCARD IterationStatus
      CustomVisitChildren(KRYS_NOESCAPE const Func<IterationStatus(css::Value &)> &func) const noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::DynamicRangeLimitValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Value &value) noexcept
  {
    return value.IsDynamicRangeLimitValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
