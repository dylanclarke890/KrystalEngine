#include "Krystal.Booey/Style/Values/UI/AccentColor.hpp"

#include "Krystal.Booey/Graphics/Animation/AnimationUtilities.hpp"

namespace krys::boo::style
{
  const Color &AccentColor::ColorOrCurrentColor() const
  {
    if (_value)
    {
      return *_value;
    }
    return Color::GetCurrentColor();
  }

  // MARK: - Blending

  bool Blending<AccentColor>::Equals(const AccentColor &a, const AccentColor &b, const RenderStyle &aStyle,
                                     const RenderStyle &bStyle) noexcept
  {
    bool aAuto = a.IsAuto();
    bool bAuto = b.IsAuto();

    if (aAuto || bAuto)
    {
      return aAuto == bAuto;
    }

    return style::EqualsForBlending(*a.TryColor(), *b.TryColor(), aStyle, bStyle);
  }

  bool Blending<AccentColor>::CanBlend(const AccentColor &a, const AccentColor &b) noexcept
  {
    return !a.IsAuto() && !b.IsAuto() && style::CanBlend(*a.TryColor(), *b.TryColor());
  }

  AccentColor Blending<AccentColor>::Blend(const AccentColor &a, const AccentColor &b,
                                           const RenderStyle &aStyle, const RenderStyle &bStyle,
                                           const BlendingContext &context) noexcept
  {
    if (context.IsDiscrete)
    {
      krys_debug_assert(!context.Progress || context.Progress == 1);
      return context.Progress ? b : a;
    }

    krys_debug_assert(CanBlend(a, b));
    return style::Blend(*a.TryColor(), *b.TryColor(), aStyle, bStyle, context);
  }
}
