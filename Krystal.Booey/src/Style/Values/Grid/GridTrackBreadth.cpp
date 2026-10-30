#include "Krystal.Booey/Style/Values/Grid/GridTrackBreadth.hpp"

#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/Graphics/Animation/AnimationUtilities.hpp"
#include "Krystal.Booey/Style/BuilderState.hpp"
#include "Krystal.Booey/Style/Values/Primitives/LengthWrapperBlending.hpp"
#include "Krystal.Booey/Style/Values/Primitives/LengthWrapperCSSValueConversion.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesBlending.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesCSSValueConversion.hpp"

namespace krys::boo::style
{
  // MARK: - Conversion

  GridTrackBreadth
    CSSValueConversion<GridTrackBreadth>::operator()(BuilderState &state,
                                                     const css::PrimitiveValue &primitiveValue) noexcept
  {
    if (primitiveValue.IsFlex())
    {
      return ConvertCSSValueToStyle<Flex<css::NonNegative>>(state, primitiveValue);
    }
    return ConvertCSSValueToStyle<GridTrackBreadthLength>(state, primitiveValue);
  }

  // MARK: - Blending

  GridTrackBreadth Blending<GridTrackBreadth>::Blend(const GridTrackBreadth &from, const GridTrackBreadth &to,
                                                     const BlendingContext &context) noexcept
  {
    if (from.IsFlex() != to.IsFlex())
    {
      return context.Progress < 0.5 ? from : to;
    }

    if (from.IsFlex())
    {
      return style::Blend(from.GetFlex(), to.GetFlex(), context);
    }
    return style::Blend(from.Length(), to.Length(), context);
  }
}
