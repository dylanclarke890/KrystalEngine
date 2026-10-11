#pragma once

#include "Krystal.Booey/CSS/Values/Background/BoxShadowProperty.hpp"
#include "Krystal.Booey/CSS/Values/Value.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  class BoxShadowPropertyValue final : public Value
  {
  private:
    BoxShadowProperty _shadow;

    BoxShadowPropertyValue(BoxShadowProperty shadow) noexcept
        : Value(ValueType::BoxShadowProperty), _shadow {krys::move(shadow)}
    {
    }

  public:
    KRYS_NODISCARD static Ref<BoxShadowPropertyValue> Create(BoxShadowProperty shadow) noexcept;

    KRYS_NODISCARD CSSOMString CustomCSSText(const SerialisationContext &context) const noexcept;

    KRYS_NODISCARD bool Equals(const BoxShadowPropertyValue &other) const noexcept;

    KRYS_NODISCARD IterationStatus
      CustomVisitChildren(const Func<IterationStatus(css::Value &)> &func) const noexcept;

    KRYS_NODISCARD const BoxShadowProperty &Shadow() const noexcept
    {
      return _shadow;
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::BoxShadowPropertyValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Value &value) noexcept
  {
    return value.IsBoxShadowPropertyValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()