#pragma once

#include "Krystal.Booey/SVG/Properties/SVGValuePropertyList.hpp"
#include "Krystal.Booey/SVG/SVGLength.hpp"

namespace krys::boo::svg
{
  /// @see https://svgwg.org/svg2-draft/types.html#InterfaceSVGLengthList
  class SVGLengthList final : public SVGValuePropertyList<SVGLength>
  {
    using Base = SVGValuePropertyList<SVGLength>;
    using Base::Base;

  private:
    SVGLengthMode _lengthMode {SVGLengthMode::Other};

  public:
    KRYS_NODISCARD static Ref<SVGLengthList> Create(SVGLengthMode lengthMode = SVGLengthMode::Other) noexcept
    {
      return AdoptRef(*new SVGLengthList(lengthMode));
    }

    KRYS_NODISCARD static Ref<SVGLengthList> Create(SVGPropertyOwner *owner, SVGPropertyAccess access,
                                                    SVGLengthMode lengthMode) noexcept
    {
      return AdoptRef(*new SVGLengthList(owner, access, lengthMode));
    }

    KRYS_NODISCARD static Ref<SVGLengthList> Create(const SVGLengthList &other,
                                                    SVGPropertyAccess access) noexcept
    {
      return AdoptRef(*new SVGLengthList(other, access));
    }

    KRYS_NODISCARD SVGLengthMode LengthMode() const noexcept
    {
      return _lengthMode;
    }

    bool Parse(dom::DOMStringView) noexcept;
    KRYS_NODISCARD dom::DOMString ValueAsString() const noexcept override;

  private:
    SVGLengthList(SVGLengthMode lengthMode) noexcept : _lengthMode(lengthMode)
    {
    }

    SVGLengthList(SVGPropertyOwner *owner, SVGPropertyAccess access, SVGLengthMode lengthMode) noexcept
        : Base(owner, access), _lengthMode(lengthMode)
    {
    }

    SVGLengthList(const SVGLengthList &other, SVGPropertyAccess access) noexcept
        : Base(other, access), _lengthMode(other.LengthMode())
    {
    }
  };
}
