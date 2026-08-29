#include "Krystal.HTML/CSS/Properties/Consumers/Box.hpp"
#include "Krystal.HTML/CSS/Parser/CSSTokenRange.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/Ident.hpp"
#include "Krystal.HTML/CSS/Values/CSSPrimitiveValue.hpp"
#include "Krystal.HTML/CSS/Values/CSSValueList.hpp"
#include "Krystal.HTML/CSS/Values/CSSValueListBuilder.hpp"
#include "Krystal.HTML/CSS/Values/Enums/CSSValueId.hpp"

namespace Krys::HTML::CSSPropertyParserHelpers
{
  RefPtr<CSSValue> ConsumeMarginTrim(CSSTokenRange &tokens, CSSPropertyParserState &state) noexcept
  {
    // <'margin-trim'> = none | [ block || inline ] | [ block-start || inline-start || block-end || inline-end
    // ] https://drafts.csswg.org/css-box/#margin-trim

    auto firstValue = tokens.Peek().ValueId();
    if (firstValue == CSSValueId::None)
    {
      return ConsumeIdent(tokens);
    }

    // FIXME: Multiple values should be appended in canonical order.
    SmallList<CSSValueId, 4uz> idents;
    if (firstValue == CSSValueId::Block || firstValue == CSSValueId::Inline)
    {
      while (auto ident = ConsumeIdentRaw<CSSValueId::Block, CSSValueId::Inline>(tokens))
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
      while (auto ident = ConsumeIdentRaw<CSSValueId::BlockStart, CSSValueId::BlockEnd,
                                          CSSValueId::InlineStart, CSSValueId::InlineEnd>(tokens))
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
        if (std::ranges::contains(idents, CSSValueId::BlockStart)
            && std::ranges::contains(idents, CSSValueId::BlockEnd))
        {
          return CSSPrimitiveValue::Create(CSSValueId::Block);
        }

        if (std::ranges::contains(idents, CSSValueId::InlineStart)
            && std::ranges::contains(idents, CSSValueId::InlineEnd))
        {
          return CSSPrimitiveValue::Create(CSSValueId::Inline);
        }
      }
      else if (idents.size() == 4uz)
      {
        CSSValueListBuilder list;
        list.push_back(CSSPrimitiveValue::Create(CSSValueId::Block));
        list.push_back(CSSPrimitiveValue::Create(CSSValueId::Inline));

        return CSSValueList::CreateSpaceSeparated(Krys::Move(list));
      }
    }

    CSSValueListBuilder list;
    for (auto ident : idents)
    {
      list.push_back(CSSPrimitiveValue::Create(ident));
    }

    return CSSValueList::CreateSpaceSeparated(Krys::Move(list));
  }
}