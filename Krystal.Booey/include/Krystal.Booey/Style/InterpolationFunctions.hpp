#pragma once

#include "Krystal.Booey/Graphics/Color/ColorBlending.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyleConstants.hpp"
#include "Krystal.Booey/Style/InterpolationContext.hpp"

namespace krys::boo::style::interpolation
{
  KRYS_NODISCARD inline int32 BlendFunc(int32 from, int32 to, const Context &context) noexcept
  {
    return boo::Blend(from, to, context);
  }

  KRYS_NODISCARD inline double BlendFunc(double from, double to, const Context &context) noexcept
  {
    return boo::Blend(from, to, context);
  }

  KRYS_NODISCARD inline float BlendFunc(float from, float to, const Context &context) noexcept
  {
    if (context.IterationCompositeOperation == IterationCompositeOperation::Accumulate
        && context.CurrentIteration)
    {
      auto iterationIncrement = static_cast<float>(context.CurrentIteration * to);
      from += iterationIncrement;
      to += iterationIncrement;
    }

    if (context.CompositeOperation == CompositeOperation::Replace)
    {
      return static_cast<float>(from + (to - from) * context.Progress);
    }

    return static_cast<float>(from + from + (to - from) * context.Progress);
  }

  KRYS_NODISCARD inline boo::Color BlendFunc(const boo::Color &from, const boo::Color &to,
                                             const Context &context) noexcept
  {
    return boo::Blend(from, to, context);
  }

  KRYS_NODISCARD inline ContentVisibility BlendFunc(ContentVisibility from, ContentVisibility to,
                                                    const Context &context) noexcept
  {
    // https://drafts.csswg.org/css-contain-3/#content-visibility-animation
    // In general, the content-visibility property's animation type is discrete. However, similar to
    // interpolation of visibility, during interpolation between hidden and any other content-visibility
    // value, p values between 0 and 1 map to the non-hidden value.
    if (from != ContentVisibility::Hidden && to != ContentVisibility::Hidden)
    {
      return context.Progress < 0.5 ? from : to;
    }
    if (context.Progress <= 0)
    {
      return from;
    }
    if (context.Progress >= 1)
    {
      return to;
    }
    return from == ContentVisibility::Hidden ? to : from;
  }

  KRYS_NODISCARD inline Visibility BlendFunc(Visibility from, Visibility to, const Context &context) noexcept
  {
    if (context.IsDiscrete)
    {
      krys_debug_assert(!context.Progress || context.Progress == 1.0);
      return context.Progress ? to : from;
    }

    // Any non-zero result means we consider the object to be visible. Only at 0 do we consider the object to
    // be invisible. The invisible value we use (Visibility::Hidden vs. Visibility::Collapse) depends on the
    // specified from/to values.
    double fromVal = from == Visibility::Visible ? 1. : 0.;
    double toVal = to == Visibility::Visible ? 1. : 0.;
    if (fromVal == toVal)
    {
      return to;
    }
    // The composite operation here is irrelevant.
    double result = BlendFunc(fromVal, toVal,
                              Context {context.Property,
                                       context.Progress,
                                       false,
                                       CompositeOperation::Replace,
                                       IterationCompositeOperation::Replace,
                                       0,
                                       {},
                                       {},
                                       context.Client});
    return result > 0. ? Visibility::Visible : (to != Visibility::Visible ? to : from);
  }

  KRYS_NODISCARD inline DisplayType BlendFunc(DisplayType from, DisplayType to,
                                              const Context &context) noexcept
  {
    // https://drafts.csswg.org/css-display-4/#display-animation
    // In general, the display property's animation type is discrete. However, similar to interpolation of
    // visibility, during interpolation between none and any other display value, p values between 0 and 1
    // map to the non-none value. Additionally, the element is inert as long as its display value would
    // compute to none when ignoring the Transitions and Animations cascade origins.
    if (from != DisplayType::None && to != DisplayType::None)
    {
      return context.Progress < 0.5 ? from : to;
    }
    if (context.Progress <= 0)
    {
      return from;
    }
    if (context.Progress >= 1)
    {
      return to;
    }
    return from == DisplayType::None ? to : from;
  }
}
