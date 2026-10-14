#include "Krystal.Booey/Style/Values/ScrollSnap/ScrollPadding.hpp"

#include "Krystal.Booey/Graphics/LayoutRect.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesEvaluation.hpp"

namespace krys::boo::style
{
  // MARK: - Evaluation

  LayoutUnit Evaluation<ScrollPaddingEdge, LayoutUnit>::operator()(const ScrollPaddingEdge &edge,
                                                                   LayoutUnit referenceLength,
                                                                   ZoomNeeded token) noexcept
  {
    return krys::SwitchOn(
      edge, [&](const ScrollPaddingEdge::Fixed &fixed) { return Evaluate<LayoutUnit>(fixed, token); },
      [&](const ScrollPaddingEdge::Percentage &percentage)
      { return Evaluate<LayoutUnit>(percentage, referenceLength); },
      [&](const ScrollPaddingEdge::Calc &calculated)
      { return Evaluate<LayoutUnit>(calculated, referenceLength, token); },
      [&](const css::keywords::Auto &) { return 0_lu; });
  }

  float Evaluation<ScrollPaddingEdge, float>::operator()(const ScrollPaddingEdge &edge, float referenceLength,
                                                         ZoomNeeded token) noexcept
  {
    return krys::SwitchOn(
      edge, [&](const ScrollPaddingEdge::Fixed &fixed) { return Evaluate<float>(fixed, token); },
      [&](const ScrollPaddingEdge::Percentage &percentage)
      { return Evaluate<float>(percentage, referenceLength); }, [&](const ScrollPaddingEdge::Calc &calculated)
      { return Evaluate<float>(calculated, referenceLength, token); },
      [&](const css::keywords::Auto &) { return 0.0f; });
  }

  KRYS_NODISCARD LayoutBoxExtent ExtentForRect(const ScrollPaddingBox &padding, const LayoutRect &rect,
                                               ZoomNeeded token) noexcept
  {
    return LayoutBoxExtent {
      Evaluate<LayoutUnit>(padding.Top(), rect.Height(), token),
      Evaluate<LayoutUnit>(padding.Right(), rect.Width(), token),
      Evaluate<LayoutUnit>(padding.Bottom(), rect.Height(), token),
      Evaluate<LayoutUnit>(padding.Left(), rect.Width(), token),
    };
  }
}
