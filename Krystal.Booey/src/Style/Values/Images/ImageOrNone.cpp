#include "Krystal.Booey/Style/Values/Images/ImageOrNone.hpp"

#include "Krystal.Booey/Graphics/Animation/AnimationUtilities.hpp"
#include "Krystal.Booey/Style/Seam.hpp"

namespace krys::boo::style
{
  // MARK: - Conversion

  ImageOrNone CSSValueConversion<ImageOrNone>::operator()(BuilderState &state,
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

  bool Blending<ImageOrNone>::CanBlend(const ImageOrNone &a, const ImageOrNone &b) noexcept
  {
    return !a.IsNone() && !b.IsNone();
  }

  ImageOrNone Blending<ImageOrNone>::Blend(const ImageOrNone &a, const ImageOrNone &b,
                                           const BlendingContext &context) noexcept
  {
    if (context.IsDiscrete)
    {
      krys_debug_assert(!context.Progress || context.Progress == 1.0);
      return context.Progress ? b : a;
    }

    krys_debug_assert(CanBlend(a, b));
    return Blending<ImageWrapper> {}.Blend(*a.TryImage(), *b.TryImage(), context);
  }
}
