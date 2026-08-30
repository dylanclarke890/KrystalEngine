#include "Krystal.HTML/CSS/Properties/Consumers/Percentage.hpp"
#include "Krystal.HTML/CSS/Parser/CSSTokenRange.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/CSSPrimitiveValue.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/NumberDefinitions.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/PercentageDefinitions.hpp"
#include "Krystal.HTML/CSS/Properties/CSSPropertyParserState.hpp"
#include "Krystal.HTML/CSS/Values/CSSPrimitiveValue.hpp"

namespace Krys::HTML::CSSPropertyParserHelpers
{
  RefPtr<CSSPrimitiveValue> ConsumePercentageDividedBy100OrNumber(CSSTokenRange &tokens,
                                                                  CSSPropertyParserState &state) noexcept
  {
    using NumberConsumer = ConsumerDefinition<Number<>>;
    using PercentageConsumer = ConsumerDefinition<Percentage<>>;

    auto &token = tokens.Peek();

    switch (token.Type())
    {
      case CSSTokenType::Function:
      {
        if (auto value = NumberConsumer::FunctionToken::Consume(tokens, state, {}, {}))
        {
          return CSSPrimitiveValueResolver<Number<>>::Resolve(*value);
        }

        if (auto value = PercentageConsumer::FunctionToken::Consume(tokens, state, {}, {}))
        {
          return CSSPrimitiveValueResolver<Percentage<>>::Resolve(*value);
        }

        break;
      }
      case CSSTokenType::Number:
      {
        if (auto value = NumberConsumer::NumberToken::Consume(tokens, state, {}, {}))
        {
          return CSSPrimitiveValueResolver<Number<>>::Resolve(*value);
        }

        break;
      }
      case CSSTokenType::Percentage:
      {
        if (auto value = PercentageConsumer::PercentageToken::Consume(tokens, state, {}, {}))
        {
          return CSSPrimitiveValue::Create(value->Value / 100.0);
        }

        break;
      }
      default:
      {
        break;
      }
    }

    return nullptr;
  }
}