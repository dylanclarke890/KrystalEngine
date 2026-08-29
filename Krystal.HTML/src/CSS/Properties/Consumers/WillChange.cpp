#include "Krystal.HTML/CSS/Properties/Consumers/WillChange.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/Ident.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/Primitives.hpp"
#include "Krystal.HTML/CSS/Properties/CSSPropertyParserState.hpp"
#include "Krystal.HTML/CSS/Values/CSSPrimitiveValue.hpp"
#include "Krystal.HTML/CSS/Values/CSSValueList.hpp"
#include "Krystal.HTML/CSS/Values/CSSValueListBuilder.hpp"

namespace Krys::HTML::CSSPropertyParserHelpers
{
  KRYS_NODISCARD RefPtr<CSSValue> ConsumeWillChange(CSSTokenRange &tokens,
                                                    CSSPropertyParserState &state) noexcept
  {
    // <'will-change'> = auto | <animateable-feature>#
    // https://drafts.csswg.org/css-will-change/#propdef-will-change

    if (tokens.Peek().ValueId() == CSSValueId::Auto)
    {
      return ConsumeIdent(tokens);
    }

    // Every comma-separated list of identifiers is a valid will-change value, unless the list includes an
    // explicitly disallowed identifier.
    CSSValueListBuilder values;
    while (!tokens.IsAtEnd())
    {
      switch (tokens.Peek().ValueId())
      {
        case CSSValueId::Contents:
        case CSSValueId::ScrollPosition:
        {
          values.push_back(ConsumeIdent(tokens));
          break;
        }
        case CSSValueId::None:
        case CSSValueId::All:
        case CSSValueId::Auto:
        {
          return nullptr;
        }
        default:
        {
          if (tokens.Peek().Type() != CSSTokenType::Ident)
          {
            return nullptr;
          }

          CSSPropertyId propertyId = FindCSSProperty(tokens.Peek().IdentCodePoints());
          if (propertyId == CSSPropertyId::WillChange)
          {
            return nullptr;
          }

          if (!IsExposed(propertyId, &state.Context.PropertySettings))
          {
            propertyId = CSSPropertyId::Invalid;
          }

          if (propertyId != CSSPropertyId::Invalid)
          {
            values.push_back(CSSPrimitiveValue::Create(propertyId));

            tokens.Discard();
            tokens.DiscardWhitespace();

            break;
          }

          if (auto customIdent = ConsumeCustomIdent(tokens))
          {
            // Append properties we don't recognize, but that are legal.
            values.push_back(Krys::Move(customIdent));
            break;
          }

          return nullptr;
        }
      }

      // This is a comma separated list
      if (!tokens.IsAtEnd() && !ConsumeComma(tokens))
      {
        return nullptr;
      }
    }

    return CSSValueList::CreateCommaSeparated(Krys::Move(values));
  }
}