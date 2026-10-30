#pragma once

#include "Krystal.Booey/CSS/Values/FilterEffects/AppleColorFilter.hpp"
#include "Krystal.Booey/CSS/Values/Value.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  /// @brief A `<'-apple-color-filter'>` value, e.g. `apple-invert-lightness() sepia(50%)`.
  class AppleColorFilterValue final : public Value
  {
  private:
    AppleColorFilter _filter;

    explicit AppleColorFilterValue(AppleColorFilter &&filter) noexcept;

  public:
    KRYS_NODISCARD static Ref<AppleColorFilterValue> Create(AppleColorFilter filter) noexcept;

    KRYS_NODISCARD const AppleColorFilter &GetFilter() const noexcept
    {
      return _filter;
    }

    KRYS_NODISCARD CSSOMString CustomCSSText(const SerialisationContext &context) const noexcept;
    KRYS_NODISCARD bool Equals(const AppleColorFilterValue &other) const noexcept;
    KRYS_NODISCARD IterationStatus
      CustomVisitChildren(KRYS_NOESCAPE const Func<IterationStatus(Value &)> &func) const noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::AppleColorFilterValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Value &value) noexcept
  {
    return value.IsAppleColorFilterValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
