#include "Krystal.Booey/Style/Values/Fonts/FontVariantNumeric.hpp"

#include "Krystal.Booey/CSS/Properties/Consumers/Font.hpp"
#include "Krystal.Booey/Style/BuilderChecking.hpp"

namespace krys::boo::style
{
  // MARK: - Conversion

  FontVariantNumeric CSSValueConversion<FontVariantNumeric>::operator()(BuilderState &state,
                                                                        const css::Value &value) noexcept
  {
    if (const auto *primitiveValue = DynamicDowncast<css::PrimitiveValue>(value))
    {
      switch (primitiveValue->ValueId())
      {
        case css::ValueId::Normal: return css::keywords::Normal {};
        default:                   state.SetCurrentPropertyInvalidAtComputedValueTime(); return css::keywords::Normal {};
      }
    }

    auto list = RequiredListDowncast<css::ValueList, css::PrimitiveValue>(state, value);
    if (!list)
    {
      return css::keywords::Normal {};
    }

    auto figure = FontVariantNumericFigure::Normal;
    auto spacing = FontVariantNumericSpacing::Normal;
    auto fraction = FontVariantNumericFraction::Normal;
    auto ordinal = FontVariantNumericOrdinal::Normal;
    auto slashedZero = FontVariantNumericSlashedZero::Normal;

    for (const auto &item : *list)
    {
      switch (item.ValueId())
      {
        case css::ValueId::LiningNums:        figure = FontVariantNumericFigure::LiningNumbers; break;
        case css::ValueId::OldstyleNums:      figure = FontVariantNumericFigure::OldStyleNumbers; break;
        case css::ValueId::ProportionalNums:  spacing = FontVariantNumericSpacing::ProportionalNumbers; break;
        case css::ValueId::TabularNums:       spacing = FontVariantNumericSpacing::TabularNumbers; break;
        case css::ValueId::DiagonalFractions: fraction = FontVariantNumericFraction::DiagonalFractions; break;
        case css::ValueId::StackedFractions:  fraction = FontVariantNumericFraction::StackedFractions; break;
        case css::ValueId::Ordinal:           ordinal = FontVariantNumericOrdinal::Yes; break;
        case css::ValueId::SlashedZero:       slashedZero = FontVariantNumericSlashedZero::Yes; break;
        default:                              state.SetCurrentPropertyInvalidAtComputedValueTime(); return css::keywords::Normal {};
      }
    }

    return FontVariantNumeric::Platform {figure, spacing, fraction, ordinal, slashedZero};
  }
}
