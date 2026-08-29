#include "Krystal.HTML/CSS/Properties/Consumers/Scrollbars.hpp"
#include "Krystal.HTML/CSS/Parser/CSSTokenRange.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/Color.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/Ident.hpp"
#include "Krystal.HTML/CSS/Properties/CSSPropertyParserState.hpp"
#include "Krystal.HTML/CSS/Values/CSSValuePair.hpp"

namespace Krys::HTML::CSSPropertyParserHelpers
{
  RefPtr<CSSValue> ConsumeScrollbarColor(CSSTokenRange &range, CSSPropertyParserState &state) noexcept
  {
    // <'scrollbar-color'> = auto | <color>{2}
    // https://drafts.csswg.org/css-scrollbars/#propdef-scrollbar-color

    if (auto ident = ConsumeIdent<CSSValueId::Auto>(range))
    {
      return ident;
    }

    if (auto thumbColor = ConsumeColor(range, state))
    {
      if (auto trackColor = ConsumeColor(range, state))
      {
        return CSSValuePair::CreateNonCoalescing(Krys::Move(thumbColor), Krys::Move(trackColor));
      }
    }

    return nullptr;
  }
}