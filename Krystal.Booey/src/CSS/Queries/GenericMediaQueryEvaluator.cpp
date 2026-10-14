#include "Krystal.Booey/CSS/Queries/GenericMediaQueryEvaluator.hpp"
#include "Krystal.Booey/CSS/ToLengthConversionData.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Values/RatioValue.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesConversions.hpp"
#include "Krystal.Booey/Style/Values/Primitives/Ratio.hpp"

namespace krys::boo::css::mq
{
  KRYS_NODISCARD static Maybe<LayoutUnit> ComputeLength(const Value *value,
                                                        const ToLengthConversionData &conversionData) noexcept
  {
    auto *primitiveValue = DynamicDowncast<PrimitiveValue>(value);
    if (!primitiveValue)
    {
      return {};
    }

    if (primitiveValue->IsNumberOrInteger())
    {
      if (primitiveValue->ResolveAsNumber(conversionData))
      {
        return {};
      }
      return 0_lu;
    }

    if (!primitiveValue->IsLength())
    {
      return {};
    }
    return primitiveValue->ResolveAsLength<LayoutUnit>(conversionData);
  }

  template <typename T>
  KRYS_NODISCARD static bool Compare(ComparisonOperator op, T left, T right) noexcept
  {
    switch (op)
    {
      case ComparisonOperator::LessThan:           return left < right;
      case ComparisonOperator::GreaterThan:        return left > right;
      case ComparisonOperator::LessThanOrEqual:    return left <= right;
      case ComparisonOperator::GreaterThanOrEqual: return left >= right;
      case ComparisonOperator::Equal:              return left == right;
    }
    krys_unreachable();
  }

  enum class Side : uint8
  {
    Left,
    Right
  };

  KRYS_NODISCARD static EvaluationResult
    EvaluateLengthComparison(LayoutUnit size, const Maybe<Comparison> &comparison, Side side,
                             const ToLengthConversionData &conversionData) noexcept
  {
    if (!comparison)
    {
      return EvaluationResult::True;
    }

    auto expressionSize = ComputeLength(comparison->ProtectedValue().get(), conversionData);
    if (!expressionSize)
    {
      return EvaluationResult::Unknown;
    }

    auto left = side == Side::Left ? *expressionSize : size;
    auto right = side == Side::Left ? size : *expressionSize;

    return ToEvaluationResult(Compare(comparison->Op, left, right));
  }

  KRYS_NODISCARD static EvaluationResult
    EvaluateNumberComparison(double number, const Maybe<Comparison> &comparison, Side side,
                             const ToLengthConversionData &conversionData) noexcept
  {
    if (!comparison)
    {
      return EvaluationResult::True;
    }

    auto expressionNumber = Downcast<PrimitiveValue>(*comparison->Value).ResolveAsNumber(conversionData);

    auto left = side == Side::Left ? expressionNumber : number;
    auto right = side == Side::Left ? number : expressionNumber;

    return ToEvaluationResult(Compare(comparison->Op, left, right));
  }

  KRYS_NODISCARD static EvaluationResult
    EvaluateIntegerComparison(int32 number, const Maybe<Comparison> &comparison, Side side,
                              const ToLengthConversionData &conversionData) noexcept
  {
    if (!comparison)
    {
      return EvaluationResult::True;
    }

    auto expressionNumber =
      Downcast<PrimitiveValue>(*comparison->Value).ResolveAsInteger<int32>(conversionData);

    auto left = side == Side::Left ? expressionNumber : number;
    auto right = side == Side::Left ? number : expressionNumber;

    return ToEvaluationResult(Compare(comparison->Op, left, right));
  }

  KRYS_NODISCARD static EvaluationResult
    EvaluateResolutionComparison(float resolution, const Maybe<Comparison> &comparison, Side side,
                                 const ToLengthConversionData &conversionData) noexcept
  {
    if (!comparison)
    {
      return EvaluationResult::True;
    }

    auto expressionResolution =
      Downcast<PrimitiveValue>(*comparison->Value).ResolveAsResolution<float>(conversionData);

    auto left = side == Side::Left ? expressionResolution : resolution;
    auto right = side == Side::Left ? resolution : expressionResolution;

    return ToEvaluationResult(Compare(comparison->Op, left, right));
  }

  EvaluationResult EvaluateLengthFeature(const Feature &feature, LayoutUnit length,
                                         const ToLengthConversionData &conversionData) noexcept
  {
    if (!feature.LeftComparison && !feature.RightComparison)
    {
      return ToEvaluationResult(!!length);
    }

    auto leftResult = EvaluateLengthComparison(length, feature.LeftComparison, Side::Left, conversionData);
    auto rightResult = EvaluateLengthComparison(length, feature.RightComparison, Side::Right, conversionData);

    return leftResult & rightResult;
  }

  KRYS_NODISCARD static EvaluationResult
    EvaluateRatioComparison(FloatSize size, const Maybe<Comparison> &comparison, Side side,
                            const ToLengthConversionData &conversionData) noexcept
  {
    if (!comparison)
    {
      return EvaluationResult::True;
    }

    auto *ratioValue = comparison->Value ? DynamicDowncast<RatioValue>(*comparison->Value) : nullptr;
    if (!ratioValue)
    {
      return EvaluationResult::Unknown;
    }

    auto resolvedRatio = style::ConvertToStyle(ratioValue->GetRatio(), conversionData);

    // Ratio with zero denominator is infinite and compares greater to any value.

    auto comparisonA = resolvedRatio.Denominator.Value ? size.Height() * resolvedRatio.Numerator.Value : 1.0f;
    auto comparisonB =
      resolvedRatio.Denominator.Value ? size.Width() * resolvedRatio.Denominator.Value : 0.0f;

    auto left = side == Side::Left ? comparisonA : comparisonB;
    auto right = side == Side::Left ? comparisonB : comparisonA;

    return ToEvaluationResult(Compare(comparison->Op, left, right));
  }

  EvaluationResult EvaluateRatioFeature(const Feature &feature, FloatSize size,
                                        const ToLengthConversionData &conversionData) noexcept
  {
    if (!feature.LeftComparison && !feature.RightComparison)
    {
      return ToEvaluationResult(size.Width());
    }

    auto leftResult = EvaluateRatioComparison(size, feature.LeftComparison, Side::Left, conversionData);
    auto rightResult = EvaluateRatioComparison(size, feature.RightComparison, Side::Right, conversionData);

    return leftResult & rightResult;
  }

  EvaluationResult EvaluateBooleanFeature(const Feature &feature, bool currentValue,
                                          const ToLengthConversionData &conversionData) noexcept
  {
    if (!feature.RightComparison)
    {
      return ToEvaluationResult(currentValue);
    }

    auto &value = Downcast<PrimitiveValue>(*feature.RightComparison->Value);
    auto expectedValue = value.ResolveAsInteger<int32>(conversionData);

    if (expectedValue && expectedValue != 1)
    {
      return EvaluationResult::Unknown;
    }

    return ToEvaluationResult(expectedValue == static_cast<int32>(currentValue));
  }

  EvaluationResult EvaluateIntegerFeature(const Feature &feature, int32 currentValue,
                                          const ToLengthConversionData &conversionData) noexcept
  {
    if (!feature.LeftComparison && !feature.RightComparison)
    {
      return ToEvaluationResult(!!currentValue);
    }

    auto leftResult =
      EvaluateIntegerComparison(currentValue, feature.LeftComparison, Side::Left, conversionData);
    auto rightResult =
      EvaluateIntegerComparison(currentValue, feature.RightComparison, Side::Right, conversionData);

    return leftResult & rightResult;
  }

  EvaluationResult EvaluateNumberFeature(const Feature &feature, double currentValue,
                                         const ToLengthConversionData &conversionData) noexcept
  {
    if (!feature.LeftComparison && !feature.RightComparison)
    {
      return ToEvaluationResult(!!currentValue);
    }

    auto leftResult =
      EvaluateNumberComparison(currentValue, feature.LeftComparison, Side::Left, conversionData);
    auto rightResult =
      EvaluateNumberComparison(currentValue, feature.RightComparison, Side::Right, conversionData);

    return leftResult & rightResult;
  }

  EvaluationResult EvaluateResolutionFeature(const Feature &feature, float currentValue,
                                             const ToLengthConversionData &conversionData) noexcept
  {
    if (!feature.LeftComparison && !feature.RightComparison)
    {
      return ToEvaluationResult(!!currentValue);
    }

    auto leftResult =
      EvaluateResolutionComparison(currentValue, feature.LeftComparison, Side::Left, conversionData);
    auto rightResult =
      EvaluateResolutionComparison(currentValue, feature.RightComparison, Side::Right, conversionData);

    return leftResult & rightResult;
  }

  EvaluationResult EvaluateIdentifierFeature(const Feature &feature, ValueId currentValue,
                                             const ToLengthConversionData &) noexcept
  {
    if (!feature.RightComparison)
    {
      return ToEvaluationResult(currentValue != ValueId::None && currentValue != ValueId::NoPreference);
    }

    auto &value = Downcast<PrimitiveValue>(*feature.RightComparison->Value);
    return ToEvaluationResult(value.ValueId() == currentValue);
  }
}
