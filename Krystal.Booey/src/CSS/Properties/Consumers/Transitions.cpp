#include "Krystal.Booey/CSS/Properties/Consumers/Transitions.hpp"
#include "Krystal.Booey/CSS/Parser/TokenRange.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Ident.hpp"
#include "Krystal.Booey/CSS/Properties/CSSPropertyParserState.hpp"
#include "Krystal.Booey/CSS/Values/CSSPrimitiveValue.hpp"

namespace krys::boo::css::CSSPropertyParserHelpers
{
  namespace
  {
    KRYS_NODISCARD RefPtr<CSSValue> ConsumeSingleTransitionPropertyIdent(TokenRange &tokens,
                                                                         const Token &token) noexcept
    {
      if (token.ValueId() == CSSValueId::All)
      {
        return ConsumeIdent(tokens);
      }

      if (auto property = token.PropertyId(); property != CSSPropertyId::Invalid)
      {
        tokens.Discard();
        tokens.DiscardWhitespace();

        return CSSPrimitiveValue::Create(property);
      }

      return ConsumeCustomIdent(tokens);
    }
  }

  RefPtr<CSSValue> ConsumeSingleTransitionPropertyOrNone(TokenRange &tokens,
                                                         CSSPropertyParserState &state) noexcept
  {
    // This variant of ConsumeSingleTransitionProperty is used for the slightly different
    // parse rules used for the 'transition' shorthand which allows 'none':
    //
    // <single-transition-or-none> = [ none | <single-transition-property> ]
    // https://drafts.csswg.org/css-transitions/#single-transition-property

    auto &token = tokens.Peek();
    if (token.Type() != TokenType::Ident)
    {
      return nullptr;
    }

    if (token.ValueId() == CSSValueId::None)
    {
      return ConsumeIdent(tokens);
    }

    return ConsumeSingleTransitionPropertyIdent(tokens, token);
  }

  RefPtr<CSSValue> ConsumeSingleTransitionProperty(TokenRange &tokens,
                                                   CSSPropertyParserState &state) noexcept
  {
    // "The <custom-ident> production in <single-transition-property> also excludes the keyword
    // none, in addition to the keywords always excluded from <custom-ident>."
    //
    // <single-transition-property> = all | <custom-ident>;
    // https://drafts.csswg.org/css-transitions/#single-transition-property

    auto &token = tokens.Peek();
    if (token.Type() != TokenType::Ident)
    {
      return nullptr;
    }

    if (token.ValueId() == CSSValueId::None)
    {
      return nullptr;
    }

    return ConsumeSingleTransitionPropertyIdent(tokens, token);
  }
}