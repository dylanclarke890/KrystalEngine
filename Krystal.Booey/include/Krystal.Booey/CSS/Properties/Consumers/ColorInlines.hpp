#pragma once

#include "Krystal.Booey/CSS/Parser/ParserFastPaths.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Color.hpp"
#include "Krystal.Booey/CSS/Values/Color/PlatformColorResolutionState.hpp"
#include "Krystal.Booey/Graphics/Color/Color.hpp"

namespace krys::boo::css::PropertyParserHelpers
{
  // MARK: <color> parsing (raw)

  KRYS_NODISCARD inline boo::Color ParseColorRawSimple(CSSOMStringView string,
                                                       const ParserContext &context) noexcept
  {
    return ParserFastPaths::ParseSimpleColor(string, context);
  }

  KRYS_NODISCARD inline boo::Color ParseColorRaw(CSSOMStringView string,
                                                 const ParserContext &context) noexcept
  {
    auto color = ParseColorRawSimple(string, context);
    if (color.IsValid())
    {
      return color;
    }

    PlatformColorResolutionState state;
    return ParseColorRawGeneral(string, context, {}, state);
  }
}
