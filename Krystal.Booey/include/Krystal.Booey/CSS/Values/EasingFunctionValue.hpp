#pragma once

#include "Krystal.Booey/CSS/Values/Easing/EasingFunction.hpp"
#include "Krystal.Booey/CSS/Values/Value.hpp"

namespace krys::boo::css
{
  class EasingFunctionValue final : public Value
  {
  public:
    KRYS_NODISCARD static Ref<EasingFunctionValue> Create(EasingFunction function) noexcept;

    KRYS_NODISCARD CSSOMString CustomCSSText(const SerialisationContext &context) const noexcept;

    KRYS_NODISCARD bool Equals(const EasingFunctionValue &other) const noexcept;

    KRYS_NODISCARD IterationStatus
      CustomVisitChildren(const Func<IterationStatus(css::Value &)> &func) const noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::EasingFunctionValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Value &value) noexcept
  {
    return value.IsEasingFunctionValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()