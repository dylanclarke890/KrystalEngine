#pragma once

#include "Krystal.Booey/CSS/Calc/SymbolTable.hpp"
#include "Krystal.Booey/CSS/Calc/Value.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/Style/BuilderChecking.hpp"
#include "Krystal.Booey/Style/Values/Primitives/LengthWrapper.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesConversions.hpp"

namespace krys::boo::style
{
  // MARK: - Conversion

  template <LengthWrapperBaseDerived T>
  struct CSSValueConversion<T>
  {
    template <typename K>
    static bool ProcessKeyword(const K &keyword, css::ValueId valueId, Maybe<T> &result) noexcept
    {
      if (valueId == keyword.Value)
      {
        result = T {keyword};
        return true;
      }

      // A few keywords have alternative spellings.
      // FIXME: Find a generic solution to this problem.
      if constexpr (SameType<K, css::keywords::MinContent>)
      {
        if (valueId == css::ValueId::WebkitMinContent)
        {
          result = T {keyword};
          return true;
        }
      }
      else if constexpr (SameType<K, css::keywords::MaxContent>)
      {
        if (valueId == css::ValueId::WebkitMaxContent)
        {
          result = T {keyword};
          return true;
        }
      }
      else if constexpr (SameType<K, css::keywords::FitContent>)
      {
        if (valueId == css::ValueId::WebkitFitContent)
        {
          result = T {keyword};
          return true;
        }
      }

      return false;
    }

    KRYS_NODISCARD static css::ToLengthConversionData
      SelectConversionData(BuilderState &builderState) noexcept
    {
      if constexpr (T::Fixed::Range.ZoomOptions == css::RangeZoomOptions::Default)
      {
        return builderState.UseSVGZoomRulesForLength()
                 ? builderState.CssToLengthConversionData().CopyWithAdjustedZoom(1.0f)
                 : builderState.CssToLengthConversionData();
      }
      else if constexpr (T::Fixed::Range.ZoomOptions == css::RangeZoomOptions::Unzoomed)
      {
        if (EvaluationTimeZoomEnabled(builderState))
        {
          return builderState.CssToLengthConversionData().CopyWithAdjustedZoom(1.0f,
                                                                               T::Fixed::Range.ZoomOptions);
        }

        return builderState.UseSVGZoomRulesForLength()
                 ? builderState.CssToLengthConversionData().CopyWithAdjustedZoom(1.0f)
                 : builderState.CssToLengthConversionData();
      }
    }

    KRYS_NODISCARD T operator()(BuilderState &state, const css::PrimitiveValue &primitiveValue) noexcept
    {
      using namespace css::Literals;

      auto convertLengthPercentage = [&]() -> T
      {
        auto conversionData = SelectConversionData(state);

        if (primitiveValue.IsLength())
        {
          return T {
            typename T::Fixed {
              css::ClampToRange<T::Fixed::Range, float>(primitiveValue.ResolveAsLength(conversionData),
                                                        css::MinValueForCssLength, css::MaxValueForCssLength),
            },
            primitiveValue.PrimitiveType() == css::UnitType::QuirkyEm};
        }

        if (primitiveValue.IsPercentage())
        {
          return T {typename T::Percentage {
            css::ClampToRange<T::Percentage::Range, float>(
              primitiveValue.ResolveAsPercentage(conversionData)),
          }};
        }

        if (primitiveValue.IsCalculatedPercentageWithLength())
        {
          return T {typename T::Calc {primitiveValue.ProtectedCSSCalcValue()->CreateCalculationValue(
            conversionData, css::SymbolTable {})}};
        }

        krys_debug_assert_msg(false, "Unexpected length-percentage value");
        state.SetCurrentPropertyInvalidAtComputedValueTime();
        return 0_css_px;
      };

      if constexpr (!T::Keywords::Count)
      {
        return convertLengthPercentage();
      }
      else
      {
        auto valueId = primitiveValue.ValueId();
        if (valueId == css::ValueId::Invalid)
        {
          return convertLengthPercentage();
        }

        constexpr auto keywordsTuple = T::Keywords::Tuple;

        auto result = std::apply(
          [&](const auto &...keyword)
          {
            Maybe<T> result;
            (ProcessKeyword(keyword, valueId, result) || ...);
            return result;
          },
          keywordsTuple);

        if (result)
        {
          return *result;
        }

        state.SetCurrentPropertyInvalidAtComputedValueTime();
        return 0_css_px;
      }
    }

    KRYS_NODISCARD T operator()(BuilderState &state, const css::Value &value) noexcept
    {
      using namespace css::Literals;

      auto *primitiveValue = RequiredDowncast<css::PrimitiveValue>(state, value);
      if (!primitiveValue)
      {
        return 0_css_px;
      }

      return (*this)(state, *primitiveValue);
    }
  };
}
