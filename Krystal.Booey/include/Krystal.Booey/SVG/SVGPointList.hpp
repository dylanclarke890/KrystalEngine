#pragma once

#include "Krystal.Booey/SVG/Properties/SVGValuePropertyList.hpp"
#include "Krystal.Booey/SVG/SVGPoint.hpp"

namespace krys::boo::svg
{
  /// @see https://svgwg.org/svg2-draft/shapes.html#InterfaceSVGPointList
  class SVGPointList final : public SVGValuePropertyList<SVGPoint>
  {
    using Base = SVGValuePropertyList<SVGPoint>;
    using Base::Base;

  public:
    KRYS_NODISCARD static Ref<SVGPointList> Create() noexcept
    {
      return AdoptRef(*new SVGPointList());
    }

    KRYS_NODISCARD static Ref<SVGPointList> Create(SVGPropertyOwner *owner, SVGPropertyAccess access) noexcept
    {
      return AdoptRef(*new SVGPointList(owner, access));
    }

    KRYS_NODISCARD static Ref<SVGPointList> Create(const SVGPointList &other,
                                                   SVGPropertyAccess access) noexcept
    {
      return AdoptRef(*new SVGPointList(other, access));
    }

    bool Parse(dom::DOMStringView) noexcept;
    KRYS_NODISCARD dom::DOMString ValueAsString() const noexcept override;
  };
}
