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

#pragma once

#include "Krystal.Booey/CSS/Parser/ParserContext.hpp"
#include "Krystal.Booey/CSS/Parser/TokenRange.hpp"
#include "Krystal.Booey/CSS/Queries/GenericMediaQueryTypes.hpp"
#include "Krystal.Core/Threading/NeverDestroyed.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Utils/SetForScope.hpp"

namespace krys::boo::css::mq
{
  /// @brief Parses a single feature, without checking it against a schema.
  struct FeatureParser
  {
    KRYS_NODISCARD static Maybe<Feature> ConsumeFeature(TokenRange &tokens,
                                                        const ParserContext &context) noexcept;
    KRYS_NODISCARD static Maybe<Feature> ConsumeBooleanOrPlainFeature(TokenRange &tokens,
                                                                      const ParserContext &context) noexcept;
    KRYS_NODISCARD static Maybe<Feature> ConsumeRangeFeature(TokenRange &tokens,
                                                             const ParserContext &context) noexcept;
    KRYS_NODISCARD static RefPtr<Value> ConsumeValue(TokenRange &tokens,
                                                     const ParserContext &context) noexcept;
    /// @brief Consumes an `<mf-comparison>` and the whitespace after it.
    KRYS_NODISCARD static Maybe<ComparisonOperator> ConsumeComparisonOperator(TokenRange &tokens) noexcept;

    /// @brief Checks the feature's syntax and values against the schema, and sets `Feature::Schema` if they
    /// match. A `<number>` given for a ratio feature is replaced by the ratio `<number> / 1`.
    KRYS_NODISCARD static bool ValidateFeatureAgainstSchema(Feature &feature,
                                                            const FeatureSchema &schema) noexcept;
  };

  /// @brief The condition grammar shared by media and container queries. `TConcreteParser` supplies the
  /// feature schemas (`FeatureSchemas()`), and may override `IsValidFunctionId` and `SchemaForFeatureName`.
  template <typename TConcreteParser>
  struct GenericMediaQueryParser
  {
    struct State
    {
      Maybe<ValueId> InFunctionId;
    };

    KRYS_NODISCARD static Maybe<Condition> ConsumeCondition(TokenRange &tokens,
                                                            const ParserContext &context) noexcept
    {
      State state {};
      return ConsumeCondition(tokens, context, state);
    }

    KRYS_NODISCARD static Maybe<Condition> ConsumeCondition(TokenRange &tokens, const ParserContext &context,
                                                            State &state) noexcept
    {
      if (tokens.Peek().Type() == TokenType::Ident)
      {
        if (tokens.Peek().ValueId() == ValueId::Not)
        {
          tokens.Discard();
          tokens.DiscardWhitespace();
          auto query = ConsumeQueryInParens(tokens, context, state);
          if (!query || !tokens.IsAtEnd())
          {
            return {};
          }

          return Condition {LogicalOperator::Not, {krys::move(*query)}};
        }
      }

      Condition condition;

      auto consumeOperator = [&]() -> Maybe<LogicalOperator>
      {
        auto &operatorToken = tokens.ConsumeIncludingWhitespace();
        if (operatorToken.Type() != TokenType::Ident)
        {
          return {};
        }
        if (operatorToken.ValueId() == ValueId::And)
        {
          return LogicalOperator::And;
        }
        if (operatorToken.ValueId() == ValueId::Or)
        {
          return LogicalOperator::Or;
        }
        return {};
      };

      do
      {
        if (!condition.Queries.empty())
        {
          auto op = consumeOperator();
          if (!op)
          {
            return {};
          }
          if (condition.Queries.size() > 1 && condition.LogicalOperator != *op)
          {
            return {};
          }
          condition.LogicalOperator = *op;
        }

        auto query = ConsumeQueryInParens(tokens, context, state);
        if (!query)
        {
          return {};
        }

        condition.Queries.push_back(krys::move(*query));
      } while (!tokens.IsAtEnd());

      return condition;
    }

    KRYS_NODISCARD static Maybe<QueryInParens>
      ConsumeQueryInParens(TokenRange &tokens, const ParserContext &context, State &state) noexcept
    {
      Maybe<ValueId> functionId;

      if (tokens.Peek().Type() == TokenType::Function)
      {
        if (state.InFunctionId)
        {
          return {};
        }

        functionId = tokens.Peek().FunctionId();
        if (!TConcreteParser::IsValidFunctionId(*functionId))
        {
          CSSOMString name {tokens.Peek().IdentCodePoints()};
          auto functionRange = tokens.ConsumeBlock();
          tokens.DiscardWhitespace();
          return GeneralEnclosed {krys::move(name),
                                  functionRange.Serialise(TokenSerialisationMode::CustomProperty)};
        }
      }

      if (!functionId && tokens.Peek().Type() != TokenType::OpenParen)
      {
        return {};
      }

      auto originalBlockRange = tokens.ConsumeBlock();
      tokens.DiscardWhitespace();

      auto blockRange = originalBlockRange;
      blockRange.DiscardWhitespace();

      SetForScope functionScope(state.InFunctionId, functionId ? functionId : state.InFunctionId);

      auto conditionRange = blockRange;
      if (auto condition = ConsumeCondition(conditionRange, context, state))
      {
        condition->FunctionId = functionId;
        return QueryInParens {krys::move(*condition)};
      }

      auto featureRange = blockRange;
      if (auto feature = ConsumeAndValidateFeature(featureRange, context, state))
      {
        feature->FunctionId = functionId;
        return QueryInParens {krys::move(*feature)};
      }

      auto styleRangeRange = blockRange;
      if (auto range = TConcreteParser::ConsumeStyleRange(styleRangeRange, context, state))
      {
        range->FunctionId = functionId;
        return QueryInParens {krys::move(*range)};
      }

      return GeneralEnclosed {functionId ? CSSOMString(ToString(*functionId)) : CSSOMString {},
                              originalBlockRange.Serialise(TokenSerialisationMode::CustomProperty)};
    }

    KRYS_NODISCARD static Maybe<Feature>
      ConsumeAndValidateFeature(TokenRange &tokens, const ParserContext &context, State &state) noexcept
    {
      auto feature = FeatureParser::ConsumeFeature(tokens, context);
      if (!feature)
      {
        return {};
      }

      if (!ValidateFeature(*feature, context, state))
      {
        return {};
      }

      return feature;
    }

    KRYS_NODISCARD static bool IsValidFunctionId(ValueId) noexcept
    {
      return false;
    }

    KRYS_NODISCARD static Maybe<StyleRange> ConsumeStyleRange(TokenRange &, const ParserContext &,
                                                              State &) noexcept
    {
      return {};
    }

    KRYS_NODISCARD static const FeatureSchema *SchemaForFeatureName(const CSSOMStringAtom &name,
                                                                    const ParserContext &, State &) noexcept
    {
      using SchemaMap = HashMap<CSSOMStringAtom, const FeatureSchema *>;

      static NeverDestroyed<SchemaMap> schemas = []
      {
        SchemaMap map;
        for (auto *entry : TConcreteParser::FeatureSchemas())
        {
          map.emplace(entry->Name, entry);
        }
        return map;
      }();

      auto it = schemas->find(name);
      return it == schemas->end() ? nullptr : it->second;
    }

    KRYS_NODISCARD static bool ValidateFeature(Feature &feature, const ParserContext &context,
                                               State &state) noexcept
    {
      auto *schema = TConcreteParser::SchemaForFeatureName(feature.Name, context, state);
      if (!schema)
      {
        return false;
      }
      return FeatureParser::ValidateFeatureAgainstSchema(feature, *schema);
    }
  };
}
