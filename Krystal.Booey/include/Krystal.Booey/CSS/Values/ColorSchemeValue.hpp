#pragma once

#include "Krystal.Booey/CSS/Values/ColorAdjust/ColorScheme.hpp"
#include "Krystal.Booey/CSS/Values/Value.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  class ColorSchemeValue : public Value
  {
  private:
    ColorScheme _scheme;

    ColorSchemeValue(ColorScheme scheme) noexcept : Value(ValueType::ColorScheme), _scheme(krys::move(scheme))
    {
    }

  public:
    KRYS_NODISCARD static Ref<ColorSchemeValue> Create(ColorScheme scheme) noexcept;

    KRYS_NODISCARD CSSOMString CustomCSSText(const SerialisationContext &context) const noexcept;

    KRYS_NODISCARD bool Equals(const ColorSchemeValue &other) const noexcept;

    KRYS_NODISCARD IterationStatus
      CustomVisitChildren(const Func<IterationStatus(css::Value &)> &func) const noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::ColorSchemeValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Value &value) noexcept
  {
    return value.IsColorSchemeValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()