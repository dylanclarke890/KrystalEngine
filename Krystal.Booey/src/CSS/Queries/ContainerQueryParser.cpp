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
