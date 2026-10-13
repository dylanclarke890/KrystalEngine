#pragma once

#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Booey/Graphics/Color/Color.hpp"
#include "Krystal.Core/Attributes.hpp"

namespace krys::boo::css
{
  struct SerialisationContext;
}

namespace krys::boo::style
{
  /// @brief `currentcolor`, resolved at use time.
  struct CurrentColor
  {
    KRYS_NODISCARD constexpr bool operator==(const CurrentColor &) const noexcept = default;
  };

  KRYS_NODISCARD inline boo::Color ResolveColor(const CurrentColor &, const boo::Color &currentColor) noexcept
  {
    return currentColor;
  }

  KRYS_NODISCARD constexpr bool ContainsCurrentColor(const CurrentColor &) noexcept
  {
    return true;
  }

  void SerialisationForCSSTokenization(css::CSSOMString &builder, const css::SerialisationContext &context,
                                       const CurrentColor &value) noexcept;
  KRYS_NODISCARD css::CSSOMString SerialisationForCSSTokenization(const css::SerialisationContext &context,
                                                                  const CurrentColor &value) noexcept;
}
