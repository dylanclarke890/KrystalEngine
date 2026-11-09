/*
 * Copyright (C) 2022 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.Booey/CSS/Queries/ContainerQueryParser.hpp"
#include "Krystal.Booey/CSS/Parser/VariableParser.hpp"
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

  // https://drafts.csswg.org/css-conditional-5/#typedef-style-range
  Maybe<mq::StyleRange> ContainerQueryParser::ConsumeStyleRange(TokenRange &tokens,
                                                                const MediaQueryParserContext &context,
                                                                State &state) noexcept
  {
    if (state.InFunctionId != ValueId::Style)
    {
      return {};
    }

    auto isComparisonDelimiter = [](const Token &token)
    {
      return token.Type() == TokenType::Delim
             && (token.Delimiter() == U'<' || token.Delimiter() == U'>' || token.Delimiter() == U'=');
    };

    // <style-range-value> = <custom-property-name> | <style-feature-value>
    auto consumeValue = [&]() -> Maybe<mq::StyleRangeValue>
    {
      auto begin = tokens;
      while (!tokens.IsAtEnd() && !isComparisonDelimiter(tokens.Peek()))
      {
        tokens.DiscardComponentValue();
      }
      auto valueRange = begin.RangeUntil(tokens);
      valueRange.DiscardTrailingWhitespace();
      if (valueRange.IsAtEnd())
      {
        return {};
      }

      auto nameRange = valueRange;
      if (nameRange.Peek().Type() == TokenType::Ident
          && IsCustomPropertyName(nameRange.Peek().IdentCodePoints()))
      {
        CSSOMStringAtom name {nameRange.Consume().IdentCodePoints()};
        if (nameRange.IsAtEnd())
        {
          return mq::StyleRangeValue {krys::move(name)};
        }
      }

      // Parsed as a custom property value, so the var() and other substitutions are resolved like those of a
      // style feature's value. The name has a NUL, which no CSS can give, so it's never a registered
      // property.
      static const CSSOMStringAtom valueName {CSSOMStringView {u8"--\0", 3uz}};
      auto value = VariableParser::ParseDeclarationValue(valueName, valueRange, context.Context);
      if (!value)
      {
        return {};
      }
      return mq::StyleRangeValue {CSSOMStringAtom::Null(), krys::move(value)};
    };

    mq::StyleRange range;
    while (true)
    {
      auto value = consumeValue();
      if (!value)
      {
        return {};
      }
      range.Values.push_back(krys::move(*value));
      if (tokens.IsAtEnd())
      {
        break;
      }
      if (range.Values.size() == 3uz)
      {
        return {};
      }
      auto op = mq::FeatureParser::ConsumeComparisonOperator(tokens);
      if (!op)
      {
        return {};
      }
      range.Operators.push_back(*op);
    }

    if (range.Values.size() < 2uz)
    {
      return {};
    }

    // Three values compare both ways with <mf-lt> or both with <mf-gt>.
    if (range.Values.size() == 3uz)
    {
      auto isLess = [](mq::ComparisonOperator op)
      {
        return op == mq::ComparisonOperator::LessThan || op == mq::ComparisonOperator::LessThanOrEqual;
      };
      auto isGreater = [](mq::ComparisonOperator op)
      {
        return op == mq::ComparisonOperator::GreaterThan || op == mq::ComparisonOperator::GreaterThanOrEqual;
      };
      bool bothLess = isLess(range.Operators[0]) && isLess(range.Operators[1]);
      bool bothGreater = isGreater(range.Operators[0]) && isGreater(range.Operators[1]);
      if (!bothLess && !bothGreater)
      {
        return {};
      }
    }

    range.Schema = &Features::Style();
    return range;
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
