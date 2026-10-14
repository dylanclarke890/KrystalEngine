#include "Krystal.Booey/Style/Values/ScrollSnap/ScrollMargin.hpp"

#include "Krystal.Booey/Graphics/LayoutRect.hpp"
#include "Krystal.Booey/Style/Seam.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesCSSValueConversion.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesEvaluation.hpp"

namespace krys::boo::style
{
  // MARK: - Conversion

  ScrollMarginEdge CSSValueConversion<ScrollMarginEdge>::operator()(BuilderState &state,
                                                                    const css::Value &value) noexcept
  {
    return ScrollMarginEdge {ConvertCSSValueToStyle<Length<>>(state, value)};
  }

  // MARK: - Evaluation

  LayoutUnit Evaluation<ScrollMarginEdge, LayoutUnit>::operator()(const ScrollMarginEdge &edge, LayoutUnit,
                                                                  ZoomNeeded token) noexcept
  {
    return Evaluate<LayoutUnit>(edge._value, token);
  }

  LayoutUnit Evaluation<ScrollMarginEdge, LayoutUnit>::operator()(const ScrollMarginEdge &edge,
                                                                  ZoomNeeded token) noexcept
  {
    return Evaluate<LayoutUnit>(edge._value, token);
  }

  float Evaluation<ScrollMarginEdge, float>::operator()(const ScrollMarginEdge &edge, float,
                                                        ZoomNeeded token) noexcept
  {
    return Evaluate<float>(edge._value, token);
  }

  float Evaluation<ScrollMarginEdge, float>::operator()(const ScrollMarginEdge &edge,
                                                        ZoomNeeded token) noexcept
  {
    return Evaluate<float>(edge._value, token);
  }

  // MARK: - Extent

  KRYS_NODISCARD LayoutBoxExtent ExtentForRect(const ScrollMarginBox &margin, const LayoutRect &rect) noexcept
  {
    return LayoutBoxExtent {
      Evaluate<LayoutUnit>(margin.Top(), rect.Height(), ZoomNeeded {}),
      Evaluate<LayoutUnit>(margin.Right(), rect.Width(), ZoomNeeded {}),
      Evaluate<LayoutUnit>(margin.Bottom(), rect.Height(), ZoomNeeded {}),
      Evaluate<LayoutUnit>(margin.Left(), rect.Width(), ZoomNeeded {}),
    };
  }
}
