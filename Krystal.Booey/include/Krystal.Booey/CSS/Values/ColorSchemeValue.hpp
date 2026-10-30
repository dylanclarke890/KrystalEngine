#pragma once

#include "Krystal.Booey/CSS/Values/ColorAdjust/ColorScheme.hpp"
#include "Krystal.Booey/CSS/Values/Value.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  /// @brief A `<'color-scheme'>` value.
  class ColorSchemeValue final : public Value
  {
  private:
    css::ColorScheme _colorScheme;

    explicit ColorSchemeValue(css::ColorScheme colorScheme) noexcept;

  public:
    KRYS_NODISCARD static Ref<ColorSchemeValue> Create(css::ColorScheme colorScheme) noexcept;

    KRYS_NODISCARD const css::ColorScheme &GetColorScheme() const noexcept
    {
      return _colorScheme;
    }

    KRYS_NODISCARD CSSOMString CustomCSSText(const SerialisationContext &context) const noexcept;
    KRYS_NODISCARD bool Equals(const ColorSchemeValue &other) const noexcept;
    KRYS_NODISCARD IterationStatus
      CustomVisitChildren(KRYS_NOESCAPE const Func<IterationStatus(css::Value &)> &func) const noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::ColorSchemeValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Value &value) noexcept
  {
    return value.IsColorSchemeValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
