#include "Krystal.Booey/Style/Values/Text/LetterSpacing.hpp"

#include "Krystal.Booey/Style/BuilderChecking.hpp"
#include "Krystal.Booey/Style/Seam.hpp"
#include "Krystal.Booey/Style/Values/Primitives/LengthWrapperCSSValueConversion.hpp"

namespace krys::boo::style
{
  LetterSpacing CSSValueConversion<LetterSpacing>::operator()(BuilderState &state,
                                                              const css::Value &value) noexcept
  {
    auto cssToLengthConversionDataWithTextZoomFactor = [](BuilderState &state) -> css::ToLengthConversionData
    {
      auto zoom = state.ZoomWithTextZoomFactor();
      if (zoom == state.CssToLengthConversionData().Zoom())
      {
        return state.CssToLengthConversionData();
      }
      return state.CssToLengthConversionData().CopyWithAdjustedZoom(zoom,
                                                                    LetterSpacing::Fixed::Range.ZoomOptions);
    };

    auto *primitiveValue = RequiredDowncast<css::PrimitiveValue>(state, value);
    if (!primitiveValue)
    {
      return css::keywords::Normal {};
    }

    if (primitiveValue->ValueId() == css::ValueId::Normal)
    {
      return css::keywords::Normal {};
    }

    auto conversionData = state.UseSVGZoomRulesForLength()
                            ? state.CssToLengthConversionData().CopyWithAdjustedZoom(1.0f)
                            : cssToLengthConversionDataWithTextZoomFactor(state);

    if (primitiveValue->IsLength())
    {
      return LetterSpacing {typename LetterSpacing::Fixed {
                              css::ClampToRange<LetterSpacing::Fixed::Range, float>(
                                primitiveValue->ResolveAsLength(conversionData), css::MinValueForCssLength,
                                css::MaxValueForCssLength),
                            },
                            primitiveValue->PrimitiveType() == css::UnitType::QuirkyEm};
    }

    if (primitiveValue->IsPercentage())
    {
      return LetterSpacing {typename LetterSpacing::Percentage {
        css::ClampToRange<LetterSpacing::Percentage::Range, float>(
          primitiveValue->ResolveAsPercentage(conversionData)),
      }};
    }

    if (primitiveValue->IsCalculatedPercentageWithLength())
    {
      return LetterSpacing {
        typename LetterSpacing::Calc {primitiveValue->ProtectedCSSCalcValue()->CreateCalculationValue(
          conversionData, css::SymbolTable {})}};
    }

    state.SetCurrentPropertyInvalidAtComputedValueTime();
    return css::keywords::Normal {};
  }
}
