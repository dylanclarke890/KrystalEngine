#pragma once

// #include "Krystal.Booey/CSS/LayoutUnit.hpp"
#include "Krystal.Booey/CSS/Queries/GenericMediaQueryTypes.hpp"
#include "Krystal.Booey/CSS/ToLengthConversionData.hpp"
#include "Krystal.Booey/CSS/Values/ValueId.hpp"

namespace krys::boo
{
  class RenderElement;
}

namespace krys::boo::css
{
  namespace mq
  {
    KRYS_NODISCARD EvaluationResult EvaluateLengthFeature(
      const Feature &feature, LayoutUnit, const ToLengthConversionData &conversionData) noexcept;

    KRYS_NODISCARD EvaluationResult EvaluateRatioFeature(
      const Feature &feature, FloatSize, const ToLengthConversionData &conversionData) noexcept;

    KRYS_NODISCARD EvaluationResult EvaluateBooleanFeature(
      const Feature &feature, bool, const ToLengthConversionData &conversionData) noexcept;

    KRYS_NODISCARD EvaluationResult EvaluateIntegerFeature(
      const Feature &feature, int, const ToLengthConversionData &conversionData) noexcept;

    KRYS_NODISCARD EvaluationResult EvaluateNumberFeature(
      const Feature &feature, double, const ToLengthConversionData &conversionData) noexcept;

    KRYS_NODISCARD EvaluationResult EvaluateResolutionFeature(
      const Feature &feature, float, const ToLengthConversionData &conversionData) noexcept;

    KRYS_NODISCARD EvaluationResult EvaluateIdentifierFeature(
      const Feature &feature, ValueId identifier, const ToLengthConversionData &conversionData) noexcept;

    template <typename ConcreteEvaluator>
    class GenericMediaQueryEvaluator
    {
    public:
      KRYS_NODISCARD EvaluationResult EvaluateQueryInParens(
        const QueryInParens &queryInParens, const FeatureEvaluationContext &context) const noexcept
      {
        return krys::SwitchOn(
          queryInParens, [&](const Condition &condition) { return EvaluateCondition(condition, context); },
          [&](const mq::Feature &feature) { return AsConcreteEvaluator().EvaluateFeature(feature, context); },
          [&](const mq::GeneralEnclosed &) { return mq::EvaluationResult::Unknown; });
      }

      KRYS_NODISCARD EvaluationResult
        EvaluateCondition(const Condition &condition, const FeatureEvaluationContext &context) const noexcept
      {
        if (condition.Queries.empty())
        {
          return EvaluationResult::Unknown;
        }

        switch (condition.LogicalOperator)
        {
          case LogicalOperator::Not:
          {
            return !AsConcreteEvaluator().EvaluateQueryInParens(condition.Queries.front(), context);
          }
          // Kleene 3-valued logic.
          case LogicalOperator::And:
          {
            auto result = EvaluationResult::True;
            for (auto &query : condition.Queries)
            {
              auto queryResult = AsConcreteEvaluator().EvaluateQueryInParens(query, context);
              if (queryResult == EvaluationResult::False)
              {
                return EvaluationResult::False;
              }

              if (queryResult == EvaluationResult::Unknown)
              {
                result = EvaluationResult::Unknown;
              }
            }

            return result;
          }
          case LogicalOperator::Or:
          {
            auto result = EvaluationResult::False;
            for (auto &query : condition.Queries)
            {
              auto queryResult = AsConcreteEvaluator().EvaluateQueryInParens(query, context);
              if (queryResult == EvaluationResult::True)
              {
                return EvaluationResult::True;
              }

              if (queryResult == EvaluationResult::Unknown)
              {
                result = EvaluationResult::Unknown;
              }
            }

            return result;
          }
        }

        krys_unreachable();
      }

      KRYS_NODISCARD EvaluationResult EvaluateFeature(const Feature &feature,
                                                      const FeatureEvaluationContext &context) const noexcept
      {
        if (!feature.Schema)
        {
          return mq::EvaluationResult::Unknown;
        }

        return feature.Schema->Evaluate(feature, context);
      }

    private:
      KRYS_NODISCARD const ConcreteEvaluator &AsConcreteEvaluator() const noexcept
      {
        return static_cast<const ConcreteEvaluator &>(*this);
      }
    };

    KRYS_NODISCARD constexpr EvaluationResult operator&(EvaluationResult left,
                                                        EvaluationResult right) noexcept
    {
      if (left == EvaluationResult::Unknown || right == EvaluationResult::Unknown)
      {
        return EvaluationResult::Unknown;
      }

      if (left == EvaluationResult::True && right == EvaluationResult::True)
      {
        return EvaluationResult::True;
      }

      return EvaluationResult::False;
    }

    KRYS_NODISCARD constexpr EvaluationResult operator!(EvaluationResult result) noexcept
    {
      switch (result)
      {
        case EvaluationResult::True:    return EvaluationResult::False;
        case EvaluationResult::False:   return EvaluationResult::True;
        case EvaluationResult::Unknown: return EvaluationResult::Unknown;
      }

      krys_unreachable();
    }

    KRYS_NODISCARD constexpr EvaluationResult ToEvaluationResult(bool boolean) noexcept
    {
      return boolean ? EvaluationResult::True : EvaluationResult::False;
    }
  }
}
