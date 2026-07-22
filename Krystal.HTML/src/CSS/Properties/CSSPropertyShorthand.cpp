#include "Krystal.HTML/CSS/Properties/CSSPropertyShorthand.hpp"

namespace Krys::HTML
{
  namespace
  {
    constexpr CSSPropertyShorthand AllShorthand() noexcept
    {
      constexpr static auto properties = std::to_array<CSSPropertyId>({
        CSSPropertyId::MarginTop,
        CSSPropertyId::MarginBottom,
        CSSPropertyId::MarginLeft,
        CSSPropertyId::MarginRight,
      });

      return CSSPropertyShorthand {
        .ShorthandId = CSSPropertyId::Margin,
        .LonghandProperties = properties,
      };
    }

    constexpr CSSPropertyShorthand MarginShorthand() noexcept
    {
      constexpr static auto properties = std::to_array<CSSPropertyId>({
        CSSPropertyId::MarginTop,
        CSSPropertyId::MarginBottom,
        CSSPropertyId::MarginLeft,
        CSSPropertyId::MarginRight,
      });

      return CSSPropertyShorthand {
        .ShorthandId = CSSPropertyId::Margin,
        .LonghandProperties = properties,
      };
    }
  }

  CSSPropertyShorthandList MatchingShorthandsForLonghand(CSSPropertyId property) noexcept
  {
    switch (property)
    {
      case CSSPropertyId::MarginTop:
      case CSSPropertyId::MarginBottom:
      case CSSPropertyId::MarginLeft:
      case CSSPropertyId::MarginRight:
      {
        return CSSPropertyShorthandList {AllShorthand(), MarginShorthand()};
      }
    }

    assert(false);
    return {};
  }
}