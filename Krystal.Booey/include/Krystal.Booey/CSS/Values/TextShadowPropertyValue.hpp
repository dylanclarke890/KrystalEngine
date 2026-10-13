#pragma once

#include "Krystal.Booey/CSS/Values/TextDecoration/TextShadowProperty.hpp"
#include "Krystal.Booey/CSS/Values/Value.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  /// @brief A `text-shadow` value: `none` or a list of `<text-shadow>`s.
  class TextShadowPropertyValue final : public Value
  {
  private:
    TextShadowProperty _shadow;

    explicit TextShadowPropertyValue(TextShadowProperty &&shadow) noexcept;

  public:
    KRYS_NODISCARD static Ref<TextShadowPropertyValue> Create(TextShadowProperty shadow) noexcept;

    KRYS_NODISCARD const TextShadowProperty &Shadow() const noexcept
    {
      return _shadow;
    }

    KRYS_NODISCARD CSSOMString CustomCSSText(const SerialisationContext &context) const noexcept;
    KRYS_NODISCARD bool Equals(const TextShadowPropertyValue &other) const noexcept;
    KRYS_NODISCARD IterationStatus
      CustomVisitChildren(KRYS_NOESCAPE const Func<IterationStatus(Value &)> &func) const noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::TextShadowPropertyValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Value &value) noexcept
  {
    return value.IsTextShadowPropertyValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
