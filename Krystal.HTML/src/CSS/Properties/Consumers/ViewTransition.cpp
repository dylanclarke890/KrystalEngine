#include "Krystal.HTML/CSS/Properties/Consumers/ViewTransition.hpp"
#include "Krystal.HTML/CSS/Parser/CSSTokenRange.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/Ident.hpp"
#include "Krystal.HTML/CSS/Values/CSSPrimitiveValue.hpp"
#include "Krystal.HTML/CSS/Values/CSSValueList.hpp"
#include "Krystal.HTML/CSS/Values/CSSValueListBuilder.hpp"

namespace Krys::HTML::CSSPropertyParserHelpers
{
  RefPtr<CSSValue> ConsumeViewTransitionTypes(CSSTokenRange &tokens, CSSPropertyParserState &) noexcept
  {
    // <'types'> = none | <custom-ident>+
    // https://www.w3.org/TR/css-view-transitions-2/#descdef-view-transition-types

    if (tokens.Peek().ValueId() == CSSValueId::None)
    {
      return ConsumeIdent(tokens);
    }

    CSSValueListBuilder list;
    do
    {
      if (tokens.Peek().ValueId() == CSSValueId::None)
      {
        return nullptr;
      }

      auto type = ConsumeCustomIdent(tokens);
      if (type == nullptr)
      {
        return nullptr;
      }

      if (type->CustomIdent().starts_with(u8"-ua-"))
      {
        return nullptr;
      }

      list.push_back(Krys::Move(type));
    } while (!tokens.IsAtEnd());

    return CSSValueList::CreateSpaceSeparated(Krys::Move(list));
  }
}