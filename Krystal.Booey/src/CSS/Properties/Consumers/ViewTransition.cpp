#include "Krystal.Booey/CSS/Properties/Consumers/ViewTransition.hpp"
#include "Krystal.Booey/CSS/Parser/TokenRange.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Ident.hpp"
#include "Krystal.Booey/CSS/Values/CSSPrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Values/CSSValueList.hpp"
#include "Krystal.Booey/CSS/Values/CSSValueListBuilder.hpp"

namespace krys::boo::css::CSSPropertyParserHelpers
{
  RefPtr<CSSValue> ConsumeViewTransitionTypes(TokenRange &tokens, CSSPropertyParserState &) noexcept
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

      list.push_back(krys::move(type));
    } while (!tokens.IsAtEnd());

    return CSSValueList::CreateSpaceSeparated(krys::move(list));
  }
}