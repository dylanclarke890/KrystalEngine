#include "Krystal.Booey/CSS/Properties/Consumers/Box.hpp"
#include "Krystal.Booey/CSS/Parser/TokenRange.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Ident.hpp"
#include "Krystal.Booey/CSS/Values/CSSPrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Values/CSSValueList.hpp"
#include "Krystal.Booey/CSS/Values/CSSValueListBuilder.hpp"
#include "Krystal.Booey/CSS/Values/ValueId.hpp"

namespace krys::boo::css::PropertyParserHelpers
{
  RefPtr<CSSValue> ConsumeMarginTrim(TokenRange &tokens, PropertyParserState &state) noexcept
  {
    // <'margin-trim'> = none | [ block || inline ] | [ block-start || inline-start || block-end || inline-end
    // ] https://drafts.csswg.org/css-box/#margin-trim

    auto firstValue = tokens.Peek().ValueId();
    if (firstValue == ValueId::None)
    {
      return ConsumeIdent(tokens);
    }

    // FIXME: Multiple values should be appended in canonical order.
    SmallList<ValueId, 4uz> idents;
    if (firstValue == ValueId::Block || firstValue == ValueId::Inline)
    {
      while (auto ident = ConsumeIdentRaw<ValueId::Block, ValueId::Inline>(tokens))
      {
        if (std::ranges::contains(idents, *ident))
        {
          return nullptr;
        }

        idents.push_back(*ident);
      }
    }
    else
    {
      while (auto ident = ConsumeIdentRaw<ValueId::BlockStart, ValueId::BlockEnd,
                                          ValueId::InlineStart, ValueId::InlineEnd>(tokens))
      {
        if (std::ranges::contains(idents, *ident))
        {
          return nullptr;
        }

        idents.push_back(*ident);
      }

      // Try to serialize into either block or inline form
      if (idents.size() == 2uz)
      {
        if (std::ranges::contains(idents, ValueId::BlockStart)
            && std::ranges::contains(idents, ValueId::BlockEnd))
        {
          return CSSPrimitiveValue::Create(ValueId::Block);
        }

        if (std::ranges::contains(idents, ValueId::InlineStart)
            && std::ranges::contains(idents, ValueId::InlineEnd))
        {
          return CSSPrimitiveValue::Create(ValueId::Inline);
        }
      }
      else if (idents.size() == 4uz)
      {
        CSSValueListBuilder list;
        list.push_back(CSSPrimitiveValue::Create(ValueId::Block));
        list.push_back(CSSPrimitiveValue::Create(ValueId::Inline));

        return CSSValueList::CreateSpaceSeparated(krys::move(list));
      }
    }

    CSSValueListBuilder list;
    for (auto ident : idents)
    {
      list.push_back(CSSPrimitiveValue::Create(ident));
    }

    return CSSValueList::CreateSpaceSeparated(krys::move(list));
  }
}