#include "Krystal.Booey/CSS/Queries/ContainerQueryParser.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyParsing.hpp"
#include "Krystal.Booey/CSS/Queries/ContainerQueryFeatures.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"

namespace krys::boo::css::cq
{
  List<const mq::FeatureSchema *> ContainerQueryParser::FeatureSchemas() noexcept
  {
    return Features::AllSchemas();
  }

  Maybe<ContainerQuery>
    ContainerQueryParser::ConsumeContainerQuery(TokenRange &tokens,
                                                const MediaQueryParserContext &context) noexcept
  {
    auto consumeName = [&]
    {
      if (tokens.Peek().Type() == TokenType::OpenParen || tokens.Peek().Type() == TokenType::Function)
      {
        return CSSOMStringAtom::Null();
      }
      auto nameValue = PropertyParsing::ConsumeSingleContainerName(tokens);
      if (auto *namePrimitive = nameValue ? DynamicDowncast<PrimitiveValue>(*nameValue) : nullptr)
      {
        return CSSOMStringAtom(namePrimitive->StringValue());
      }
      return CSSOMStringAtom::Null();
    };

    auto name = consumeName();

    auto condition = ConsumeCondition(tokens, context);

    if (!condition)
    {
      if (name == CSSOMStringAtom::Null() || name == CSSOMStringAtom::Empty())
      {
        return {};
      }
      // it's valid to have a named container query without a condition, like "@container --name {}"
      condition = mq::Condition {};
    }

    auto requiredAxes = Axis::None;
    auto containsUnknownFeature = ContainsUnknownFeature::No;

    mq::TraverseFeatures(*condition,
                         [&](const mq::Feature &feature)
                         {
                           requiredAxes |= RequiredAxesForFeature(feature);
                           if (!feature.Schema)
                           {
                             containsUnknownFeature = ContainsUnknownFeature::Yes;
                           }
                         });

    return ContainerQuery {name, krys::move(*condition), requiredAxes, containsUnknownFeature};
  }

  Maybe<ContainerQueryList>
    ContainerQueryParser::ConsumeContainerQueryList(TokenRange &tokens,
                                                    const MediaQueryParserContext &context) noexcept
  {
    ContainerQueryList queries;
    while (true)
    {
      auto begin = tokens;
      while (!tokens.IsAtEnd() && tokens.Peek().Type() != TokenType::Comma)
      {
        tokens.DiscardComponentValue();
      }

      auto subrange = begin.RangeUntil(tokens);
      subrange.DiscardWhitespace();
      auto query = ConsumeContainerQuery(subrange, context);
      subrange.DiscardWhitespace();
      if (!query || !subrange.IsAtEnd())
      {
        return {};
      }
      queries.push_back(krys::move(*query));

      if (tokens.IsAtEnd())
      {
        return queries;
      }
      tokens.Discard();
    }
  }

  bool ContainerQueryParser::IsValidFunctionId(ValueId functionId) noexcept
  {
    return functionId == ValueId::Style;
  }

  const mq::FeatureSchema *ContainerQueryParser::SchemaForFeatureName(const CSSOMStringAtom &name,
                                                                      const MediaQueryParserContext &context,
                                                                      State &state) noexcept
  {
    if (state.InFunctionId == ValueId::Style)
    {
      return &Features::Style();
    }

    return GenericMediaQueryParser<ContainerQueryParser>::SchemaForFeatureName(name, context, state);
  }
}
