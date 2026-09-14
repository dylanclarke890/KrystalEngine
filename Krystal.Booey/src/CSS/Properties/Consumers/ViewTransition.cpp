#include "Krystal.Booey/CSS/Properties/Consumers/ViewTransition.hpp"
#include "Krystal.Booey/CSS/Parser/TokenRange.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Ident.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Values/ValueList.hpp"
#include "Krystal.Booey/CSS/Values/ValueListBuilder.hpp"

namespace krys::boo::css::PropertyParserHelpers
{
  RefPtr<Value> ConsumeViewTransitionTypes(TokenRange &tokens, PropertyParserState &) noexcept
  {
    // <'types'> = none | <custom-ident>+
    // https://www.w3.org/TR/css-view-transitions-2/#descdef-view-transition-types

    if (tokens.Peek().ValueId() == ValueId::None)
    {
      return ConsumeIdent(tokens);
    }

    ValueListBuilder list;
    do
    {
      if (tokens.Peek().ValueId() == ValueId::None)
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

    return ValueList::CreateSpaceSeparated(krys::move(list));
  }
}