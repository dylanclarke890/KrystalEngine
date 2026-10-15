#include "Krystal.Booey/Style/Values/TextDecoration/TextUnderlineOffset.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"

#include "Krystal.Booey/Graphics/Animation/AnimationUtilities.hpp"
#include "Krystal.Booey/Style/Seam.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesEvaluation.hpp"

namespace krys::boo::style
{
  float TextUnderlineOffset::Resolve(const RenderStyle &style, float autoValue) const noexcept
  {
    return krys::SwitchOn(
      *this, [&](const css::keywords::Auto &) -> float { return autoValue; }, [&](const Fixed &fixed) -> float
      { return style::Evaluate<float>(fixed, style.UsedZoomForLength()); }, [&](const Calc &calc) -> float
      { return style::Evaluate<float>(calc, style.ComputedFontSize(), style.UsedZoomForLength()); },
      [&](const Percentage &percentage) -> float
      { return style::Evaluate<float>(percentage, style.ComputedFontSize()); });
  }

  // MARK: - Blending

  bool Blending<TextUnderlineOffset>::CanBlend(const TextUnderlineOffset &a, const TextUnderlineOffset &b,
                                               const RenderStyle &aStyle, const RenderStyle &bStyle) noexcept
  {
    if (a.IsAuto() || b.IsAuto())
    {
      return false;
    }

    return a.Resolve(aStyle) != b.Resolve(bStyle);
  }

  TextUnderlineOffset Blending<TextUnderlineOffset>::Blend(const TextUnderlineOffset &a,
                                                           const TextUnderlineOffset &b,
                                                           const RenderStyle &aStyle,
                                                           const RenderStyle &bStyle,
                                                           const BlendingContext &context) noexcept
  {
    if (context.IsDiscrete)
    {
      krys_debug_assert(!context.Progress || context.Progress == 1.0);
      return context.Progress ? b : a;
    }

    return TextUnderlineOffset {
      TextUnderlineOffset::Fixed {boo::Blend(a.Resolve(aStyle), b.Resolve(bStyle), context)}};
  }
}
