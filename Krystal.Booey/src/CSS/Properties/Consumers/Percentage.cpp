#include "Krystal.Booey/CSS/Properties/Consumers/Percentage.hpp"
#include "Krystal.Booey/CSS/Parser/TokenRange.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/CSSPrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/NumberDefinitions.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/PercentageDefinitions.hpp"
#include "Krystal.Booey/CSS/Properties/CSSPropertyParserState.hpp"
#include "Krystal.Booey/CSS/Values/CSSPrimitiveValue.hpp"

namespace krys::boo::css::CSSPropertyParserHelpers
{
  RefPtr<CSSPrimitiveValue> ConsumePercentageDividedBy100OrNumber(TokenRange &tokens,
                                                                  CSSPropertyParserState &state) noexcept
  {
    using NumberConsumer = ConsumerDefinition<Number<>>;
    using PercentageConsumer = ConsumerDefinition<Percentage<>>;

    auto &token = tokens.Peek();

    switch (token.Type())
    {
      case TokenType::Function:
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
      case TokenType::Number:
      {
        if (auto value = NumberConsumer::NumberToken::Consume(tokens, state, {}, {}))
        {
          return CSSPrimitiveValueResolver<Number<>>::Resolve(*value);
        }

        break;
      }
      case TokenType::Percentage:
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