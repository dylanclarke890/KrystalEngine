#pragma once

#include "Krystal.Booey/SVG/Properties/SVGPropertyTraits.hpp"

namespace krys::boo::svg
{
  enum SVGZoomAndPanType : uint8
  {
    SVGZoomAndPanUnknown,
    SVGZoomAndPanDisable,
    SVGZoomAndPanMagnify
  };

  template <>
  struct SVGPropertyTraits<SVGZoomAndPanType>
  {
    KRYS_NODISCARD static SVGZoomAndPanType InitialValue() noexcept
    {
      return SVGZoomAndPanMagnify;
    }

    KRYS_NODISCARD static dom::DOMString ToString(SVGZoomAndPanType) noexcept
    {
      return {};
    }

    KRYS_NODISCARD static SVGZoomAndPanType FromString(SVGElement &, dom::DOMStringView value) noexcept
    {
      if (value == u8"disable")
      {
        return SVGZoomAndPanDisable;
      }
      if (value == u8"magnify")
      {
        return SVGZoomAndPanMagnify;
      }
      return SVGZoomAndPanUnknown;
    }
  };
}
