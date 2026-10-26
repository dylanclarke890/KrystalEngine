#pragma once

#include "Krystal.Booey/SVG/Properties/SVGValuePropertyList.hpp"
#include "Krystal.Booey/SVG/SVGNumber.hpp"

namespace krys::boo::svg
{
  /// @see https://svgwg.org/svg2-draft/types.html#InterfaceSVGNumberList
  class SVGNumberList final : public SVGValuePropertyList<SVGNumber>
  {
    using Base = SVGValuePropertyList<SVGNumber>;
    using Base::Base;

  public:
    KRYS_NODISCARD static Ref<SVGNumberList> Create() noexcept
    {
      return AdoptRef(*new SVGNumberList());
    }

    KRYS_NODISCARD static Ref<SVGNumberList> Create(SVGPropertyOwner *owner,
                                                    SVGPropertyAccess access) noexcept
    {
      return AdoptRef(*new SVGNumberList(owner, access));
    }

    KRYS_NODISCARD static Ref<SVGNumberList> Create(const SVGNumberList &other,
                                                    SVGPropertyAccess access) noexcept
    {
      return AdoptRef(*new SVGNumberList(other, access));
    }

    bool Parse(dom::DOMStringView) noexcept;
    KRYS_NODISCARD dom::DOMString ValueAsString() const noexcept override;
  };
}
