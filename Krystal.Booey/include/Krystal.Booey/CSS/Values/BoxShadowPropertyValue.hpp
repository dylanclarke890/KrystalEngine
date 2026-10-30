#pragma once

#include "Krystal.Booey/CSS/Values/Borders/BoxShadowProperty.hpp"
#include "Krystal.Booey/CSS/Values/Value.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  /// @brief A `box-shadow` value: `none` or a list of `<box-shadow>`s.
  class BoxShadowPropertyValue final : public Value
  {
  private:
    BoxShadowProperty _shadow;

    explicit BoxShadowPropertyValue(BoxShadowProperty &&shadow) noexcept;

  public:
    KRYS_NODISCARD static Ref<BoxShadowPropertyValue> Create(BoxShadowProperty shadow) noexcept;

    KRYS_NODISCARD const BoxShadowProperty &Shadow() const noexcept
    {
      return _shadow;
    }

    KRYS_NODISCARD CSSOMString CustomCSSText(const SerialisationContext &context) const noexcept;
    KRYS_NODISCARD bool Equals(const BoxShadowPropertyValue &other) const noexcept;
    KRYS_NODISCARD IterationStatus
      CustomVisitChildren(KRYS_NOESCAPE const Func<IterationStatus(Value &)> &func) const noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::BoxShadowPropertyValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Value &value) noexcept
  {
    return value.IsBoxShadowPropertyValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
