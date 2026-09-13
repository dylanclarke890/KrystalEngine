#include "Krystal.Booey/CSS/Properties/Consumers/Scrollbars.hpp"
#include "Krystal.Booey/CSS/Parser/TokenRange.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Color.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Ident.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyParserState.hpp"
#include "Krystal.Booey/CSS/Values/CSSValuePair.hpp"

namespace krys::boo::css::PropertyParserHelpers
{
  RefPtr<CSSValue> ConsumeScrollbarColor(TokenRange &range, PropertyParserState &state) noexcept
  {
    // <'scrollbar-color'> = auto | <color>{2}
    // https://drafts.csswg.org/css-scrollbars/#propdef-scrollbar-color

    if (auto ident = ConsumeIdent<ValueId::Auto>(range))
    {
      return ident;
    }

    if (auto thumbColor = ConsumeColor(range, state))
    {
      if (auto trackColor = ConsumeColor(range, state))
      {
        return CSSValuePair::CreateNonCoalescing(krys::move(thumbColor), krys::move(trackColor));
      }
    }

    return nullptr;
  }
}