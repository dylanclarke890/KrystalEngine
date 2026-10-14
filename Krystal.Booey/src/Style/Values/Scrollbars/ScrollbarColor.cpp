#include "Krystal.Booey/Style/Values/Scrollbars/ScrollbarColor.hpp"

#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Values/ValuePair.hpp"
#include "Krystal.Booey/Graphics/Animation/AnimationUtilities.hpp"
#include "Krystal.Booey/Style/BuilderChecking.hpp"
#include "Krystal.Booey/Style/ForVisitedLink.hpp"
#include "Krystal.Booey/Style/Seam.hpp"

namespace krys::boo::style
{
  // MARK: - Conversion

  ScrollbarColor CSSValueConversion<ScrollbarColor>::operator()(BuilderState &state,
                                                                const css::Value &value) noexcept
  {
    if (css::IsValueId(value, css::ValueId::Auto))
    {
      return css::keywords::Auto {};
    }

    auto *pair = RequiredDowncast<css::ValuePair>(state, value);
    if (!pair)
    {
      return css::keywords::Auto {};
    }

    const auto &thumb = pair->First();
    const auto &track = pair->Second();

    return ScrollbarColor::Parts {
      .Thumb = ConvertCSSValueToStyle<Color>(state, thumb, style::ForVisitedLink::No),
      .Track = ConvertCSSValueToStyle<Color>(state, track, style::ForVisitedLink::No),
    };
  }

  // MARK: - Blending

  bool Blending<ScrollbarColor>::Equals(const ScrollbarColor &a, const ScrollbarColor &b,
                                        const RenderStyle &aStyle, const RenderStyle &bStyle) noexcept
  {
    bool aAuto = a.IsAuto();
    bool bAuto = b.IsAuto();

    if (aAuto || bAuto)
    {
      return aAuto == bAuto;
    }

    return style::EqualsForBlending(a.TryValue()->Thumb, b.TryValue()->Thumb, aStyle, bStyle)
           && style::EqualsForBlending(a.TryValue()->Track, b.TryValue()->Track, aStyle, bStyle);
  }

  bool Blending<ScrollbarColor>::CanBlend(const ScrollbarColor &a, const ScrollbarColor &b) noexcept
  {
    return !a.IsAuto() && !b.IsAuto() && style::CanBlend(a.TryValue()->Thumb, b.TryValue()->Thumb)
           && style::CanBlend(a.TryValue()->Track, b.TryValue()->Track);
  }

  ScrollbarColor Blending<ScrollbarColor>::Blend(const ScrollbarColor &a, const ScrollbarColor &b,
                                                 const RenderStyle &aStyle, const RenderStyle &bStyle,
                                                 const BlendingContext &context) noexcept
  {
    if (!style::CanBlend(a, b))
    {
      krys_debug_assert(!context.Progress || context.Progress == 1.0);
      return context.Progress ? b : a;
    }

    return ScrollbarColor::Parts {
      .Thumb = style::Blend(a.TryValue()->Thumb, b.TryValue()->Thumb, aStyle, bStyle, context),
      .Track = style::Blend(a.TryValue()->Track, b.TryValue()->Track, aStyle, bStyle, context),
    };
  }
}
