#include "Krystal.Booey/Style/Values/CSS2/ZIndex.hpp"

#include "Krystal.Booey/Graphics/Animation/AnimationUtilities.hpp"
#include "Krystal.Booey/Style/BuilderState.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesBlending.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesCSSValueConversion.hpp"

namespace krys::boo::style
{
  // MARK: - Conversion

  ZIndex CSSValueConversion<ZIndex>::operator()(BuilderState &state, const css::Value &value) noexcept
  {
    if (value.ValueId() == css::ValueId::Auto)
    {
      return css::keywords::Auto {};
    }
    return ConvertCSSValueToStyle<ZIndex::Value>(state, value);
  }

  // MARK: - Blending

  bool Blending<ZIndex>::CanBlend(const ZIndex &a, const ZIndex &b) noexcept
  {
    return !a.IsAuto() && !b.IsAuto();
  }

  ZIndex Blending<ZIndex>::Blend(const ZIndex &a, const ZIndex &b, const BlendingContext &context) noexcept
  {
    if (context.IsDiscrete)
    {
      krys_debug_assert(!context.Progress || context.Progress == 1);
      return context.Progress ? b : a;
    }

    krys_debug_assert(!a.IsAuto());
    krys_debug_assert(!b.IsAuto());
    return {krys::boo::style::Blend(*a.TryValue(), *b.TryValue(), context)};
  }
}
