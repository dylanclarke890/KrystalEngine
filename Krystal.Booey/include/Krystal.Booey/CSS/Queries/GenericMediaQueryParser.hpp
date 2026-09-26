#pragma once

#include "Krystal.Booey/CSS/Parser/ParserContext.hpp"
#include "Krystal.Booey/CSS/Parser/TokenRange.hpp"
#include "Krystal.Booey/CSS/Queries/GenericMediaQueryTypes.hpp"
#include "Krystal.Booey/CSS/Queries/MediaQueryParserContext.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Utils/NeverDestroyed.hpp"
#include "Krystal.Core/Utils/SetForScope.hpp"

namespace krys::boo::css::mq
{
  struct MediaQueryParserContext;

  struct FeatureParser
  {
    KRYS_NODISCARD static Maybe<Feature> ConsumeFeature(TokenRange &tokens,
                                                        const MediaQueryParserContext &context) noexcept;

    KRYS_NODISCARD static Maybe<Feature>
      ConsumeBooleanOrPlainFeature(TokenRange &tokens, const MediaQueryParserContext &context) noexcept;
    KRYS_NODISCARD static Maybe<Feature> ConsumeRangeFeature(TokenRange &tokens,
                                                             const MediaQueryParserContext &context) noexcept;

    KRYS_NODISCARD static RefPtr<Value> ConsumeValue(TokenRange &tokens,
                                                     const MediaQueryParserContext &context) noexcept;

    KRYS_NODISCARD static bool ValidateFeatureAgainstSchema(Feature &feature,
                                                            const FeatureSchema &schema) noexcept;
  };

  template <typename ConcreteParser>
  struct GenericMediaQueryParser
  {
    struct State
    {
      Maybe<ValueId> InFunctionId;
    };

    KRYS_NODISCARD static Maybe<Condition> ConsumeCondition(TokenRange &range,
                                                            const MediaQueryParserContext &context) noexcept
    {
      State state {};
      return ConsumeCondition(range, context, state);
    }

    KRYS_NODISCARD static Maybe<Condition>
      ConsumeCondition(TokenRange &tokens, const MediaQueryParserContext &context, State &state) noexcept
    {
      if (tokens.Peek().Type() == TokenType::Ident)
      {
        if (tokens.Peek().ValueId() == ValueId::Not)
        {
          tokens.ConsumeIncludingWhitespace();

          auto query = ConsumeQueryInParens(tokens, context, state);
          if (!query || !tokens.IsAtEnd())
          {
            return {};
          }

          return Condition {LogicalOperator::Not, {*query}};
        }
      }

      Condition condition;

      auto consumeOperator = [&]() -> std::optional<LogicalOperator>
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

        condition.Queries.push_back(*query);
      } while (!tokens.IsAtEnd());

      return condition;
    }

    KRYS_NODISCARD static Maybe<QueryInParens>
      ConsumeQueryInParens(TokenRange &tokens, const MediaQueryParserContext &context, State &state) noexcept
    {
      Maybe<ValueId> functionId;

      if (tokens.Peek().Type() == TokenType::Function)
      {
        if (state.InFunctionId)
        {
          return {};
        }

        functionId = tokens.Peek().FunctionId();
        if (!ConcreteParser::IsValidFunctionId(*functionId))
        {
          auto name = tokens.Peek().IdentCodePoints();
          auto functionRange = tokens.ConsumeBlock();

          tokens.DiscardWhitespace();
          return GeneralEnclosed {name, functionRange.Serialise()};
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

      SetForScope functionScope(state.InFunctionId, functionId ? *functionId : state.InFunctionId);

      auto conditionRange = blockRange;
      if (auto condition = ConsumeCondition(conditionRange, context, state))
      {
        condition->FunctionId = functionId;
        return {condition};
      }

      auto featureRange = blockRange;
      if (auto feature = ConsumeAndValidateFeature(featureRange, context, state))
      {
        feature->FunctionId = functionId;
        return {*feature};
      }

      return GeneralEnclosed {functionId ? CSSOMStringAtom(ToString(*functionId)) : CSSOMStringAtom::Null(),
                              originalBlockRange.Serialise()};
    }

    KRYS_NODISCARD static Maybe<Feature> ConsumeAndValidateFeature(TokenRange &tokens,
                                                                   const MediaQueryParserContext &context,
                                                                   State &state) noexcept
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

    KRYS_NODISCARD static bool IsValidFunctionId(ValueId functionId) noexcept
    {
      (void)functionId;
      return false;
    }

    KRYS_NODISCARD static const FeatureSchema *SchemaForFeatureName(const CSSOMStringAtom &name,
                                                                    const MediaQueryParserContext &context,
                                                                    State &state) noexcept
    {
      using SchemaMap = HashMap<CSSOMStringAtom, const FeatureSchema *>;

      static NeverDestroyed<SchemaMap> schemas = [&]
      {
        SchemaMap map;
        for (auto &entry : ConcreteParser::FeatureSchemas())
        {
          map.insert(entry->Name, entry);
        }
        return map;
      }();

      auto it = schemas->find(name);
      if (it == schemas->end())
      {
        return nullptr;
      }

      return it->second;
    }

    KRYS_NODISCARD static bool ValidateFeature(Feature &feature, const MediaQueryParserContext &context,
                                               State &state) noexcept
    {
      auto *schema = ConcreteParser::SchemaForFeatureName(feature.Name, context.Context, state);
      if (!schema)
      {
        return false;
      }

      return FeatureParser::ValidateFeatureAgainstSchema(feature, *schema);
    }
  };
}
