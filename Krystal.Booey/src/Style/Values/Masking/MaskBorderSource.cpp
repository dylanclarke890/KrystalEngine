#include "Krystal.Booey/Style/Values/Masking/MaskBorderSource.hpp"

#include "Krystal.Booey/CSS/Values/Value.hpp"
#include "Krystal.Booey/Graphics/Animation/AnimationUtilities.hpp"
#include "Krystal.Booey/Style/BuilderState.hpp"

namespace krys::boo::style
{
  // MARK: - Conversion

  MaskBorderSource CSSValueConversion<MaskBorderSource>::operator()(BuilderState &state,
                                                                    const css::Value &value) noexcept
  {
    if (value.ValueId() == css::ValueId::None)
    {
      return css::keywords::None {};
    }

    auto image = state.CreateStyleImage(value);
    if (!image)
    {
      return css::keywords::None {};
    }

    return ImageWrapper {Ref<StyleImage> {krys::move(image)}};
  }

  // MARK: - Blending

  bool Blending<MaskBorderSource>::CanBlend(const MaskBorderSource &a, const MaskBorderSource &b) noexcept
  {
    return !a.IsNone() && !b.IsNone();
  }

  MaskBorderSource Blending<MaskBorderSource>::Blend(const MaskBorderSource &a, const MaskBorderSource &b,
                                                     const BlendingContext &context) noexcept
  {
    if (context.IsDiscrete)
    {
      krys_debug_assert(!context.Progress || context.Progress == 1);
      return context.Progress ? b : a;
    }

    krys_debug_assert(CanBlend(a, b));
    return style::Blend(*a.TryImage(), *b.TryImage(), context);
  }
}
