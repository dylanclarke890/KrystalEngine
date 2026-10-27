#pragma once

#include "Krystal.Booey/Graphics/Color/Color.hpp"
#include "Krystal.Booey/SVG/Properties/SVGAnimatedPropertyImpl.hpp"
#include "Krystal.Booey/SVG/SVGElement.hpp"

namespace krys::boo::svg
{
  /// @brief The `<stop>` element: a gradient's colour at an offset.
  /// @see https://svgwg.org/svg2-draft/pservers.html#InterfaceSVGStopElement
  class SVGStopElement final : public SVGElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(SVGStopElement);

  private:
    Ref<SVGAnimatedNumber> _offset;

  public:
    explicit SVGStopElement(dom::Document &document) noexcept;

    KRYS_NODISCARD Color StopColorIncludingOpacity() const noexcept;

    KRYS_NODISCARD float Offset() const noexcept
    {
      return _offset->CurrentValue();
    }
    KRYS_NODISCARD SVGAnimatedNumber &OffsetAnimated() noexcept
    {
      return *_offset;
    }

    using PropertyRegistry = SVGPropertyOwnerRegistry<SVGStopElement, SVGElement>;

  private:
    void AttributeChanged(const dom::QualifiedName &name, Maybe<dom::DOMStringView> oldValue,
                          Maybe<dom::DOMStringView> value) noexcept final;
    void SVGAttributeChanged(const dom::QualifiedName &attributeName) noexcept final;

    KRYS_NODISCARD RenderPtr<RenderElement> CreateElementRenderer(RenderStyle &&) noexcept final;
    KRYS_NODISCARD bool RendererIsNeeded(const RenderStyle &) noexcept final;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::svg::SVGStopElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::svg::SVGElement &target) noexcept
  {
    return target.IsSVGStopElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
