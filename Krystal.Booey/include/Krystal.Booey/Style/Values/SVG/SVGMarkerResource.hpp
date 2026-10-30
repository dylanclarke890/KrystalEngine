#pragma once

#include "Krystal.Booey/Style/Values/Primitives/Url.hpp"

namespace krys::boo::style
{
  // <paint> = none | <url>
  // NOTE: `context-fill` and `context-stroke` are not implemented.
  // https://svgwg.org/svg2-draft/painting.html#SpecifyingPaint
  struct SVGMarkerResource : css::ValueOrKeyword<URL, css::keywords::None>
  {
    using Base::Base;

    KRYS_NODISCARD bool IsNone() const noexcept
    {
      return IsKeyword();
    }
    KRYS_NODISCARD bool IsURL() const noexcept
    {
      return IsValue();
    }
    KRYS_NODISCARD Maybe<URL> TryURL() const noexcept
    {
      return TryValue();
    }
  };
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::SVGMarkerResource)
