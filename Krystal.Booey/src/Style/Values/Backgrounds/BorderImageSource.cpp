#include "Krystal.Booey/Style/Values/Backgrounds/BorderImageSource.hpp"

#include "Krystal.Booey/CSS/Values/Value.hpp"
#include "Krystal.Booey/Graphics/Animation/AnimationUtilities.hpp"
#include "Krystal.Booey/Style/Seam.hpp"

namespace krys::boo::style
{
  // MARK: - Conversion

  BorderImageSource CSSValueConversion<BorderImageSource>::operator()(BuilderState &state,
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

  bool Blending<BorderImageSource>::CanBlend(const BorderImageSource &a, const BorderImageSource &b) noexcept
  {
    return !a.IsNone() && !b.IsNone();
  }

  BorderImageSource Blending<BorderImageSource>::Blend(const BorderImageSource &a, const BorderImageSource &b,
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
