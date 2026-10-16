#pragma once

#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Booey/Graphics/Color/Color.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

namespace krys::boo::css
{
  struct ResolvedColor;
  struct SerialisationContext;
}

namespace krys::boo::style
{
  struct Color;
  struct ColorResolutionState;

  /// @brief A colour that's fully resolved to a platform colour.
  struct ResolvedColor
  {
    boo::Color Value;

    KRYS_NODISCARD bool operator==(const ResolvedColor &) const noexcept = default;
  };

  KRYS_NODISCARD Color ToStyleColor(const css::ResolvedColor &unresolved,
                                    ColorResolutionState &state) noexcept;

  KRYS_NODISCARD inline boo::Color ResolveColor(const ResolvedColor &absoluteColor,
                                                const boo::Color &) noexcept
  {
    return absoluteColor.Value;
  }

  KRYS_NODISCARD constexpr bool ContainsCurrentColor(const ResolvedColor &) noexcept
  {
    return false;
  }

  void SerialisationForCSSTokenization(StringBuilder &builder, const css::SerialisationContext &context,
                                       const ResolvedColor &value) noexcept;
  KRYS_NODISCARD css::CSSOMString SerialisationForCSSTokenization(const css::SerialisationContext &context,
                                                                  const ResolvedColor &value) noexcept;
}
