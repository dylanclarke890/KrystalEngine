#pragma once

#include "Krystal.Booey/CSS/Values/Filter/AppleColorFilter.hpp"
#include "Krystal.Booey/CSS/Values/Value.hpp"

namespace krys::boo::css
{
  class AppleColorFilterValue final : public Value
  {
  private:
    AppleColorFilter _filter;

    explicit AppleColorFilterValue(AppleColorFilter filter) noexcept;

  public:
    KRYS_NODISCARD static Ref<AppleColorFilterValue> Create(AppleColorFilter filter) noexcept;

    KRYS_NODISCARD const AppleColorFilter &Filter() const noexcept
    {
      return _filter;
    }

    KRYS_NODISCARD CSSOMString CustomCSSText(const SerialisationContext &context) const noexcept;

    KRYS_NODISCARD bool Equals(const AppleColorFilterValue &other) const noexcept;

    KRYS_NODISCARD IterationStatus
      CustomVisitChildren(const Func<IterationStatus(css::Value &)> &func) const noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::AppleColorFilterValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Value &value) noexcept
  {
    return value.IsAppleColorFilterValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()