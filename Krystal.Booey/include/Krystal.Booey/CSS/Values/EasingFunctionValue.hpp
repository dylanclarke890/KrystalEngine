#pragma once

#include "Krystal.Booey/CSS/Values/Easing/EasingFunction.hpp"
#include "Krystal.Booey/CSS/Values/Value.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  /// @brief An `<easing-function>` value, e.g. `ease-in` or `steps(4, jump-start)`.
  class EasingFunctionValue final : public Value
  {
  private:
    boo::css::EasingFunction _easingFunction;

    explicit EasingFunctionValue(boo::css::EasingFunction &&easingFunction) noexcept;

  public:
    KRYS_NODISCARD static Ref<EasingFunctionValue> Create(boo::css::EasingFunction easingFunction) noexcept;

    KRYS_NODISCARD const boo::css::EasingFunction &GetEasingFunction() const noexcept
    {
      return _easingFunction;
    }

    KRYS_NODISCARD CSSOMString CustomCSSText(const SerialisationContext &context) const noexcept;
    KRYS_NODISCARD bool Equals(const EasingFunctionValue &other) const noexcept;
    KRYS_NODISCARD IterationStatus
      CustomVisitChildren(KRYS_NOESCAPE const Func<IterationStatus(Value &)> &func) const noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::EasingFunctionValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Value &value) noexcept
  {
    return value.IsEasingFunctionValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
