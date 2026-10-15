#pragma once

#include "Krystal.Booey/Style/Values/Backgrounds/BorderImageOutset.hpp"
#include "Krystal.Booey/Style/Values/Backgrounds/BorderImageRepeat.hpp"
#include "Krystal.Booey/Style/Values/Backgrounds/BorderImageSlice.hpp"
#include "Krystal.Booey/Style/Values/Backgrounds/BorderImageSource.hpp"
#include "Krystal.Booey/Style/Values/Backgrounds/BorderImageWidth.hpp"

namespace krys::boo::style
{
  // <'border-image'> = <'border-image-source'> || <'border-image-slice'> [ / <'border-image-width'> | /
  // <'border-image-width'>? / <'border-image-outset'> ]? || <'border-image-repeat'>
  // https://drafts.csswg.org/css-backgrounds/#propdef-border-image
  struct BorderImage
  {
    BorderImageSource Source;
    BorderImageSlice Slice;
    BorderImageWidth Width;
    BorderImageOutset Outset;
    BorderImageRepeat Repeat;

    BorderImage() noexcept;
    BorderImage(BorderImageSource &&, BorderImageSlice &&, BorderImageWidth &&, BorderImageOutset &&,
                BorderImageRepeat &&) noexcept;

    KRYS_NODISCARD bool operator==(const BorderImage &) const noexcept = default;
  };

  // MARK: - Conversion

  template <>
  struct CSSValueCreation<BorderImage>
  {
    KRYS_NODISCARD Ref<css::Value> operator()(css::ValuePool &, const RenderStyle &,
                                              const BorderImage &) noexcept;
  };

  // MARK: - Serialization

  template <>
  struct Serialiser<BorderImage>
  {
    void operator()(css::CSSOMString &, const css::SerialisationContext &, const RenderStyle &,
                    const BorderImage &) noexcept;
  };
}
