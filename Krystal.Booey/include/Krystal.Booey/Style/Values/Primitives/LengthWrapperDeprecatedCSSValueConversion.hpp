#pragma once

#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/Style/Values/Primitives/LengthWrapperCSSValueConversion.hpp"

namespace krys::boo::style
{
  // MARK: - Deprecated Conversions

  KRYS_NODISCARD Maybe<css::ToLengthConversionData>
    DeprecatedLengthConversionCreateCSSToLengthConversionData(RefPtr<dom::Element>) noexcept;

  template <LengthWrapperBaseDerived T>
  struct DeprecatedCSSValueConversion<T>
  {
    KRYS_NODISCARD Maybe<T> operator()(const RefPtr<dom::Element> &element,
                                       const css::PrimitiveValue &primitiveValue) noexcept
    {
      auto convertLengthPercentage = [&]() -> Maybe<T>
      {
        auto conversionData = DeprecatedLengthConversionCreateCSSToLengthConversionData(element);
        if (!conversionData)
        {
          if (primitiveValue.IsCalculated())
          {
            return null;
          }

          if (primitiveValue.IsPx())
          {
            return T {typename T::Fixed {
                        css::ClampToRange<T::Fixed::Range, float>(
                          primitiveValue.ResolveAsLengthNoConversionDataRequired(), css::MinValueForCssLength,
                          css::MaxValueForCssLength),
                      },
                      primitiveValue.PrimitiveType() == css::UnitType::QuirkyEm};
          }

          if (primitiveValue.IsPercentage())
          {
            return T {typename T::Percentage {
              css::ClampToRange<T::Percentage::Range, float>(
                primitiveValue.ResolveAsPercentageNoConversionDataRequired()),
            }};
          }

          return null;
        }

        if (primitiveValue.IsLength())
        {
          return T {
            typename T::Fixed {
              css::ClampToRange<T::Fixed::Range, float>(primitiveValue.ResolveAsLength(*conversionData),
                                                        css::MinValueForCssLength, css::MaxValueForCssLength),
            },
            primitiveValue.PrimitiveType() == css::UnitType::QuirkyEm};
        }

        if (primitiveValue.IsPercentage())
        {
          return T {typename T::Percentage {
            css::ClampToRange<T::Percentage::Range, float>(
              primitiveValue.ResolveAsPercentage(*conversionData)),
          }};
        }

        if (primitiveValue.IsCalculatedPercentageWithLength())
        {
          return T {typename T::Calc {primitiveValue.ProtectedCSSCalcValue()->CreateCalculationValue(
            *conversionData, css::SymbolTable {})}};
        }

        return null;
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
            (CSSValueConversion<T>::ProcessKeyword(keyword, valueId, result) || ...);
            return result;
          },
          keywordsTuple);

        return result;
      }
    }

    KRYS_NODISCARD Maybe<T> operator()(const RefPtr<dom::Element> &element, const css::Value &value) noexcept
    {
      auto *primitiveValue = DynamicDowncast<css::PrimitiveValue>(value);
      if (!primitiveValue)
      {
        return null;
      }

      return (*this)(element, *primitiveValue);
    }
  };
}
