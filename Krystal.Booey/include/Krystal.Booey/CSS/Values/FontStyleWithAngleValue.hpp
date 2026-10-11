#pragma once

#include "Krystal.Booey/CSS/Values/Value.hpp"

namespace krys::boo::css
{
  class FontStyleWithAngleValue : public Value
  {
  public:
    KRYS_NODISCARD CSSOMString CustomCSSText(const SerialisationContext &context) const noexcept;

    KRYS_NODISCARD bool Equals(const FontStyleWithAngleValue &other) const noexcept;

    KRYS_NODISCARD IterationStatus
      CustomVisitChildren(const Func<IterationStatus(css::Value &)> &func) const noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::FontStyleWithAngleValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Value &value) noexcept
  {
    return value.IsFontStyleWithAngleValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()