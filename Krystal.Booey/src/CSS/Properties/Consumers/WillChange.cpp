#include "Krystal.Booey/CSS/Properties/Consumers/WillChange.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Ident.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Primitives.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyParserState.hpp"
#include "Krystal.Booey/CSS/Values/CSSPrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Values/CSSValueList.hpp"
#include "Krystal.Booey/CSS/Values/CSSValueListBuilder.hpp"

namespace krys::boo::css::PropertyParserHelpers
{
  KRYS_NODISCARD RefPtr<CSSValue> ConsumeWillChange(TokenRange &tokens,
                                                    PropertyParserState &state) noexcept
  {
    // <'will-change'> = auto | <animateable-feature>#
    // https://drafts.csswg.org/css-will-change/#propdef-will-change

    if (tokens.Peek().ValueId() == ValueId::Auto)
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
        case ValueId::Contents:
        case ValueId::ScrollPosition:
        {
          values.push_back(ConsumeIdent(tokens));
          break;
        }
        case ValueId::None:
        case ValueId::All:
        case ValueId::Auto:
        {
          return nullptr;
        }
        default:
        {
          if (tokens.Peek().Type() != TokenType::Ident)
          {
            return nullptr;
          }

          PropertyId propertyId = FindProperty(tokens.Peek().IdentCodePoints());
          if (propertyId == PropertyId::WillChange)
          {
            return nullptr;
          }

          if (!IsExposed(propertyId, &state.Context.PropertySettings))
          {
            propertyId = PropertyId::Invalid;
          }

          if (propertyId != PropertyId::Invalid)
          {
            values.push_back(CSSPrimitiveValue::Create(propertyId));

            tokens.Discard();
            tokens.DiscardWhitespace();

            break;
          }

          if (auto customIdent = ConsumeCustomIdent(tokens))
          {
            // Append properties we don't recognize, but that are legal.
            values.push_back(krys::move(customIdent));
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

    return CSSValueList::CreateCommaSeparated(krys::move(values));
  }
}