#include "Krystal.Booey/CSS/Properties/Consumers/Percentage.hpp"
#include "Krystal.Booey/CSS/Parser/TokenRange.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/PrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/NumberDefinitions.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/PercentageDefinitions.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyParserState.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"

namespace krys::boo::css::PropertyParserHelpers
{
  RefPtr<PrimitiveValue> ConsumePercentageDividedBy100OrNumber(TokenRange &tokens,
                                                                  PropertyParserState &state) noexcept
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
          return PrimitiveValueResolver<Number<>>::Resolve(*value);
        }

        if (auto value = PercentageConsumer::FunctionToken::Consume(tokens, state, {}, {}))
        {
          return PrimitiveValueResolver<Percentage<>>::Resolve(*value);
        }

        break;
      }
      case TokenType::Number:
      {
        if (auto value = NumberConsumer::NumberToken::Consume(tokens, state, {}, {}))
        {
          return PrimitiveValueResolver<Number<>>::Resolve(*value);
        }

        break;
      }
      case TokenType::Percentage:
      {
        if (auto value = PercentageConsumer::PercentageToken::Consume(tokens, state, {}, {}))
        {
          return PrimitiveValue::Create(value->Value / 100.0);
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